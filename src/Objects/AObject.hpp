//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_AOBJECT_HPP
#define BATTLE_AOBJECT_HPP


#include "IObject.hpp"
#include "../Resources/Sprite.hpp"
#include "../Resources/FrameData.hpp"

namespace Battle
{
	class AObject : public IObject {
	protected:
		std::map<unsigned, std::vector<FrameData>> _moves;
		std::vector<unsigned> _allocatedTextures;
		Sprite _sprite;
		unsigned _action;

		virtual void _loadFrameData() = 0;

	public:
		AObject();
		~AObject() override;
		void render() const override;
		void update() override;
	};
}


#endif //BATTLE_AOBJECT_HPP
