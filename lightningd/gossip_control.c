#include "gossip_control.h"
#include "lightningd.h"
#include "peer_control.h"
#include "subd.h"
#include <ccan/err/err.h>
#include <ccan/take/take.h>
#include <daemon/jsonrpc.h>
#include <daemon/log.h>
#include <inttypes.h>
#include <lightningd/cryptomsg.h>
#include <lightningd/gossip/gen_gossip_wire.h>
#include <lightningd/gossip_msg.h>
#include <wire/gen_peer_wire.h>

static void gossip_finished(struct subd *gossip, int status)
{
	if (WIFEXITED(status))
		errx(1, "Gossip failed (exit status %i), exiting.",
		     WEXITSTATUS(status));
	errx(1, "Gossip failed (signal %u), exiting.", WTERMSIG(status));
}

static void peer_bad_message(struct subd *gossip, const u8 *msg)
{
	u64 unique_id;
	struct peer *peer;
	u8 *err;

	if (!fromwire_gossipstatus_peer_bad_msg(msg, msg, NULL,
						&unique_id, &err))
		fatal("Gossip gave bad PEER_BAD message %s", tal_hex(msg, msg));

	peer = peer_by_unique_id(gossip->ld, unique_id);
	if (!peer)
		fatal("Gossip gave bad peerid %"PRIu64, unique_id);

	log_debug(gossip->log, "Peer %s gave bad msg %s",
		  type_to_string(msg, struct pubkey, peer->id),
		  tal_hex(msg, msg));
	peer_set_condition(peer, "Bad message %s during gossip phase",
			   gossip_wire_type_name(fromwire_peektype(msg)));
	tal_free(peer);
}

static void peer_nongossip(struct subd *gossip, const u8 *msg, int fd)
{
	u64 unique_id;
	struct peer *peer;
	u8 *inner;
	struct crypto_state cs;

	if (!fromwire_gossipstatus_peer_nongossip(msg, msg, NULL,
						  &unique_id, &cs, &inner))
		fatal("Gossip gave bad PEER_NONGOSSIP message %s",
		      tal_hex(msg, msg));

	peer = peer_by_unique_id(gossip->ld, unique_id);
	if (!peer)
		fatal("Gossip gave bad peerid %"PRIu64, unique_id);

	if (peer->owner != gossip)
		fatal("Gossip gave bad peerid %"PRIu64" (owner %s)",
		      unique_id, peer->owner ? peer->owner->name : "(none)");

	/* It returned the fd. */
	assert(peer->fd == -1);
	peer->fd = fd;

	peer_set_condition(peer, "Gossip ended up receipt of %s",
			   wire_type_name(fromwire_peektype(inner)));

	peer_accept_open(peer, &cs, inner);
}

static void peer_ready(struct subd *gossip, const u8 *msg, int fd)
{
	u64 unique_id;
	struct peer *peer;

	if (!fromwire_gossipstatus_peer_ready(msg, NULL, &unique_id))
		fatal("Gossip gave bad PEER_READY message %s",
		      tal_hex(msg, msg));

	peer = peer_by_unique_id(gossip->ld, unique_id);
	if (!peer)
		fatal("Gossip gave bad peerid %"PRIu64, unique_id);

	log_debug_struct(gossip->log, "Peer %s ready for channel open",
			 struct pubkey, peer->id);

	if (peer->connect_cmd) {
		struct json_result *response;
		response = new_json_result(peer->connect_cmd);

		json_object_start(response, NULL);
		json_add_pubkey(response, "id", peer->id);
		json_object_end(response);
		command_success(peer->connect_cmd, response);
		peer->connect_cmd = NULL;
	}

	peer->gossip_client_fd = fd;

	peer_set_condition(peer, "Exchanging gossip");
}

static enum subd_msg_ret gossip_msg(struct subd *gossip,
				    const u8 *msg, int fd)
{
	enum gossip_wire_type t = fromwire_peektype(msg);

	switch (t) {
	/* We don't get told about fatal errors. */
	case WIRE_GOSSIPSTATUS_INIT_FAILED:
	case WIRE_GOSSIPSTATUS_BAD_NEW_PEER_REQUEST:
	case WIRE_GOSSIPSTATUS_BAD_REQUEST:
	case WIRE_GOSSIPSTATUS_FDPASS_FAILED:
	case WIRE_GOSSIPSTATUS_BAD_RELEASE_REQUEST:
	/* These are messages we send, not them. */
	case WIRE_GOSSIPCTL_NEW_PEER:
	case WIRE_GOSSIPCTL_RELEASE_PEER:
	case WIRE_GOSSIP_GETNODES_REQUEST:
	/* This is a reply, so never gets through to here. */
	case WIRE_GOSSIPCTL_RELEASE_PEER_REPLY:
	case WIRE_GOSSIP_GETNODES_REPLY:
		break;
	case WIRE_GOSSIPSTATUS_PEER_BAD_MSG:
		peer_bad_message(gossip, msg);
		break;
	case WIRE_GOSSIPSTATUS_PEER_NONGOSSIP:
		if (fd == -1)
			return SUBD_NEED_FD;
		peer_nongossip(gossip, msg, fd);
		break;
	case WIRE_GOSSIPSTATUS_PEER_READY:
		if (fd == -1) {
			return SUBD_NEED_FD;
		}
		peer_ready(gossip, msg, fd);
		break;
	}
	return SUBD_COMPLETE;
}

void gossip_init(struct lightningd *ld)
{
	ld->gossip = new_subd(ld, ld, "lightningd_gossip", NULL,
			      gossip_wire_type_name,
			      gossip_msg, gossip_finished, -1);
	if (!ld->gossip)
		err(1, "Could not subdaemon gossip");
}

static bool json_getnodes_reply(struct subd *gossip, const u8 *reply,
				struct command *cmd)
{
	u8 *inner;
	const u8 *cursor;
	size_t max;

	struct json_result *response = new_json_result(cmd);
	fromwire_gossip_getnodes_reply(reply, reply, NULL, &inner);
	max = tal_len(inner);
	cursor = inner;
	json_object_start(response, NULL);
	json_array_start(response, "nodes");

	while (max > 0) {
		struct gossip_getnodes_entry *entry = tal(reply, struct gossip_getnodes_entry);
		fromwire_gossip_getnodes_entry(&cursor, &max, entry);
		json_object_start(response, NULL);
		json_add_pubkey(response, "nodeid", &entry->nodeid);
		if (tal_len(entry->hostname) > 0) {
			json_add_string(response, "hostname", entry->hostname);
		} else {
			json_add_null(response, "hostname");
		}
		json_add_num(response, "port", entry->port);
		json_object_end(response);
		tal_free(entry);
	}
	json_array_end(response);
	json_object_end(response);
	command_success(cmd, response);
	tal_free(reply);
	return true;
}

static void json_getnodes(struct command *cmd, const char *buffer,
			  const jsmntok_t *params)
{
	struct lightningd *ld = ld_from_dstate(cmd->dstate);
	u8 *req = towire_gossip_getnodes_request(cmd);
	subd_req(ld->gossip, req, -1, NULL, json_getnodes_reply, cmd);
}

static const struct json_command getnodes_command = {
    "getnodes", json_getnodes, "Retrieve all nodes in our local network view",
    "Returns a list of all nodes that we know about"};
AUTODATA(json_command, &getnodes_command);
