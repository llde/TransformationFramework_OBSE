#include <string>
#include <forward_list>
#include "obse/GameForms.h"
#include "obse/GameAPI.h"

enum TransformType {
	Transform_Automatic = 0,
	Transform_Manual,
	Transform_Trigger //Trigger transform manaully, untrasform auto.
};

enum TransformStatus {
	StatusNormal = 0,
	StatusTransformed,
	//These below are valid only in Transform_Manual case
	StatusTransform, //TRigger transforming in the mainloop
	StatusUnTransform //trigger untrasforming in the mainloop
};

struct Transformation {
	std::string name;
	TransformType type;
	std::string skeletonSubs;
	std::string modelSubs;   //TODO representing correctly skeleton and animation substitution, and equipment subst
	std::forward_list<UInt8> b_controls;

};
	
struct TransformNPC{
	UInt8 ESPID;
	UInt32 RefID;
	TESObjectREFR* actorRef;
	Transformation* transform;
	TransformStatus status;
	UInt8 isPlayer;
};

bool InitTransformations();
