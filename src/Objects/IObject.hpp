//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_IOBJECT_HPP
#define BATTLE_IOBJECT_HPP


class IObject {
public:
	virtual ~IObject() = default;
	virtual void render() const = 0;
	virtual void update() = 0;
	virtual void reset() = 0;
	virtual bool isDead() const = 0;
};


#endif //BATTLE_IOBJECT_HPP
