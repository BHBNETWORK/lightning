#include <bitcoin/pubkey.h>

struct gossip_getnodes_entry {
	struct pubkey nodeid;
	char *hostname;
	u16 port;
};

void fromwire_gossip_getnodes_entry(const u8 **pptr, size_t *max, struct gossip_getnodes_entry *entry);
void towire_gossip_getnodes_entry(u8 **pptr, const struct gossip_getnodes_entry *entry);
