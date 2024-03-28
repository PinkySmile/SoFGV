//
// Created by PinkySmile on 10/02/24.
//

#include "ObjectFactory.hpp"
#include "Objects/Characters/SubObject.hpp"
#include "Game.hpp"

namespace SpiralOfFate
{
	size_t ObjectFactory::getObjectSize(const IObject &obj)
	{
		//                    Class ID
		size_t result = sizeof(unsigned char) + obj.getBufferSize();

		switch (obj.getClassId()) {
		case SubObject::CLASS_ID:
			//           Owner          Subobj ID
			result += sizeof(bool) + sizeof(unsigned);
			break;
		case ParticleGenerator::CLASS_ID:
			//            Owner                     Target                 Source(own)           Source(id)
			result += sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned);
			break;
		case Particle::CLASS_ID:
			//            Owner                    Source(own)         Source(gen_id)       Source(id)
			result += sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned) + sizeof(unsigned);
			break;
		}
		return result;
	}

	void ObjectFactory::saveObject(void *data, const IObject &obj)
	{
		char *ptr = (char *)data;

		*(unsigned char *)ptr = obj.getClassId();
		ptr += sizeof(unsigned char);
		switch (obj.getClassId()) {
		case SubObject::CLASS_ID: {
			assert_exp(dynamic_cast<const SubObject *>(&obj));

			auto o = reinterpret_cast<const SubObject *>(&obj);

			*(bool *)ptr = o->getOwner();
			ptr += sizeof(bool);
			*(unsigned *)ptr = o->getId();
			ptr += sizeof(unsigned);
			break;
		}
		case ParticleGenerator::CLASS_ID: {
			assert_exp(dynamic_cast<const ParticleGenerator *>(&obj));

			auto o = reinterpret_cast<const ParticleGenerator *>(&obj);

			*(unsigned char *)ptr = o->getOwner().getTeam();
			ptr += sizeof(unsigned char);
			*(unsigned char *)ptr = o->getTarget().getTeam();
			ptr += sizeof(unsigned char);
			*(unsigned char *)ptr = std::get<0>(o->getSource());
			ptr += sizeof(unsigned char);
			*(unsigned *)ptr = std::get<1>(o->getSource());
			ptr += sizeof(unsigned);
			break;
		}
		case Particle::CLASS_ID: {
			assert_exp(dynamic_cast<const Particle *>(&obj));

			auto o = reinterpret_cast<const Particle *>(&obj);

			*(unsigned char *)ptr = o->getOwner().getTeam();
			ptr += sizeof(unsigned char);
			*(unsigned char *)ptr = std::get<0>(o->getSource());
			ptr += sizeof(unsigned char);
			*(unsigned *)ptr = std::get<1>(o->getSource());
			ptr += sizeof(unsigned);
			*(unsigned *)ptr = std::get<2>(o->getSource());
			ptr += sizeof(unsigned);
			break;
		}}
		obj.copyToBuffer(ptr);
	}

	std::shared_ptr<IObject> ObjectFactory::createObject(BattleManager &mgr, void *data, const std::pair<Character *, Character *> players)
	{
		std::shared_ptr<IObject> obj;
		auto ptr = (char *)data;
		auto cl = *(unsigned char *)ptr;

		ptr += sizeof(unsigned char);
		switch (cl) {
		case Object::CLASS_ID:
			obj.reset(new Object());
			break;
		case Character::CLASS_ID:
			obj.reset(new Character());
			break;
		case SubObject::CLASS_ID: {
			auto owner = *(bool *)ptr;

			ptr += sizeof(bool);

			auto subobjid = *(unsigned *)ptr;

			ptr += sizeof(unsigned);
			obj = (&players.first)[owner]->_spawnSubObject(mgr, subobjid, false).second;
			break;
		}
		case ParticleGenerator::CLASS_ID: {
			auto owner = *(unsigned char *)ptr;
			ptr += sizeof(unsigned char);
			auto target = *(unsigned char *)ptr;
			ptr += sizeof(unsigned char);
			auto spawner = *(unsigned char *)ptr;
			ptr += sizeof(unsigned char);
			auto index = *(unsigned *)ptr;
			ptr += sizeof(unsigned);

			assert_exp(spawner <= 2);
			auto &gens =
				spawner == 2 ?
				mgr._systemParticles :
				(&players.first)[spawner]->_generators;
			assert_exp(index < gens.size());
			auto &genDat = gens[index];

			obj = std::make_shared<ParticleGenerator>(
				ParticleGenerator::Source{spawner, index},
				genDat,
				*(&players.first)[owner],
				*(&players.first)[target],
				false
			);
			break;
		}
		case Particle::CLASS_ID: {
			auto owner = *(unsigned char *) ptr;
			ptr += sizeof(unsigned char);
			auto spawner = *(unsigned char *) ptr;
			ptr += sizeof(unsigned char);
			auto genIndex = *(unsigned *) ptr;
			ptr += sizeof(unsigned);
			auto index = *(unsigned *) ptr;
			ptr += sizeof(unsigned);

			assert_exp(spawner <= 2);
			auto &gens =
				spawner == 2 ?
				mgr._systemParticles :
				(&players.first)[spawner]->_generators;
			assert_exp(genIndex < gens.size());
			auto &genDat = gens[genIndex];

			assert_exp(index < genDat.particles.size());
			obj = std::make_shared<Particle>(
				Particle::Source{spawner, genIndex, index},
				genDat.particles[index],
				*(&players.first)[owner],
				genDat.sprite,
				Vector2f{0, 0},
				false
			);
			break;
		}
		default:
			assert_not_reached_msg("Invalid class id in buffer: " + std::to_string(cl));
		}

		obj->restoreFromBuffer((void *)ptr);
		return obj;
	}
}