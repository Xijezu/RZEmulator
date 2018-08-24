#ifndef PACKETS_TS_CS_LEARN_SKILL_H
#define PACKETS_TS_CS_LEARN_SKILL_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_LEARN_SKILL_DEF(_) \
	_(simple)(uint32_t, target) \
	_(simple)(int32_t, skill_id) \
	_(simple)(uint32_t, base_random_skill_id, version >= EPIC_8_2) \
	_(simple)(int16_t, skill_level, version >= EPIC_4_1_1) // not in epic 4.0

CREATE_PACKET(TS_CS_LEARN_SKILL, 402);

/*
 * When a pet is random based (not classical fixed pet skills and stat) :
 * the summon id is determined (probably based on monsterresource.taming_id) and the summon resource matching the id is retrieved This will define the type of the pet (heal/tank/dps/ranged)
 * skill_treeX_id (in summonresource) define all the available skills.
 * In the skilltreeresource, skill_id indicate the type of skill. For random pet and when skill_group_id is not 0, its a pseudo skill (with a ? as icon)
 * When clicking "+" to learn a pseudo skill to lv1, the skillid is found in skilltree and the skill_group_id is retrieved. In summonrandomskill the matching sid is found.
 * The real skill learnt is then choosen randomly in summonrandomskill using one skill_id_X.
 *
 * In the packet, the skill_id is the known skill_id from the skilltree. So its a pseudoskill when the skill is not learnt yet and the pet is random.
 * When the skill is already at lv1 or more or when the pet is a classic one, skill_id is the real skill.
 * When the skill is already at lv1 or more and the pet is random, base_random_skill_id is set to the original pseudoskill.
 */

#endif // PACKETS_TS_CS_LEARN_SKILL_H
