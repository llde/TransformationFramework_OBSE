#include "Transformation.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"


std::string transf_path = "Data\\OBSE\\Plugins\\Transformations";
std::string skeletons_path = "Data\\Meshes\\Characters\\_male\\";

static std::vector<Transformation*>* transform_list = NULL;
static std::forward_list<TransformNPC*>*  npc_list = NULL;

InterfaceManager* interface_singleton = InterfaceManager::GetSingleton();

PlayerCharacter* playerChar = *g_thePlayer;

void HandleTransforms(void) {
	if (interface_singleton->IsGameMode() == 0) return; //Don't operate the mainloop if not in GameMode
	for each (TransformNPC* var in *npc_list){
		if (!var->isPlayer) _MESSAGE("[WARN] NON Player still not handled");
		if (var->transform->type == TransformType::Transform_Automatic) _MESSAGE("[WARN] Automatic transformation still not handled\n");
		if (var->transform->type == TransformType::Transform_Trigger) _MESSAGE("[WARN] Trigger transformation still not handled\n");
		_MESSAGE("Start transform\n");
		std::string s = skeletons_path + var->transform->skeletonSubs;
		var->oldNpc.old_skeleton = ((TESNPC*)playerChar->baseForm)->model.GetModelPath();
		if (playerChar->SetSkeletonPath(s.data()) == false) {
			_MESSAGE("[ERROR] Transformation cannot complete. Could not set Skeleton");
			//TODO remove from the list of NPC transform.
			continue;
		}
		_MESSAGE("End transform");
	}
}


bool InitTransformations(void) {
	transform_list = new std::vector<Transformation*>();
	npc_list = new std::forward_list<TransformNPC*>();
	//TODO initialize transformations reading from a file.
	//Hardcoding for now
	Transformation* werewolf = new Transformation();
	werewolf->name = "werewolf";
	werewolf->skeletonSubs = "skeleton_werewolf.nif";  //Relative to Data\Meshes\Characters\_male
	transform_list->push_back(werewolf);
}

void RegisterWerewolf(TESObjectREFR* actor){
	TESNPC* act = (TESNPC*)Oblivion_DynamicCast(actor->GetBaseForm(),0, RTTI_TESForm, RTTI_TESNPC,0);
	if(!act) return;
	TransformNPC* tr = new TransformNPC();
	tr->transform = transform_list->at(0);
	//TODO only one transformation present.
	//TODO prevent registrating an already present actor.
	tr->ESPID = actor->GetModIndex();
	tr->RefID = actor->refID;
	tr->actorRef = actor;
	if (tr->ESPID == 00 && tr->RefID == 0x000014) {
		tr->isPlayer = true;
		_MESSAGE("[INFO] Registering the player\n");
	}
	else {
		tr->isPlayer = false;
		_MESSAGE("[INFO] Registerin NPC\n");
	}
	npc_list->push_front(tr);
}

void UnregisterWerewolf(TESObjectREFR* actor) {
	TESNPC* act = (TESNPC*)Oblivion_DynamicCast(actor->GetBaseForm(), 0, RTTI_TESForm, RTTI_TESNPC, 0);
	if (!act) return;
	TransformNPC* tr_act = NULL;
	for each (TransformNPC* var in *npc_list) {
		if (var->ESPID == actor->GetModIndex() && var->RefID == actor->refID) {
			tr_act = var;
			break;
		}
	}
	npc_list->remove(tr_act);
//TODO enque and defer untransform before freeing resources
}


bool Transform(TESObjectREFR* actor) {
	TESNPC* act = (TESNPC*)Oblivion_DynamicCast(actor->GetBaseForm(), 0, RTTI_TESForm, RTTI_TESNPC, 0);
	if (!act) return false;
	TransformNPC* tr_act = NULL;
	for each (TransformNPC* var in *npc_list) {
		if (var->ESPID == actor->GetModIndex() && var->RefID == actor->refID) {
			tr_act = var;
			break;
		}
	}
	if (tr_act == NULL)  return false;   //Reference is unregistered
	tr_act->status = TransformStatus::StatusTransform;  //Enqueue the Reference for transforming 
	return true;
}


bool UnTransform(TESObjectREFR* actor) {
	TESNPC* act = (TESNPC*)Oblivion_DynamicCast(actor->GetBaseForm(), 0, RTTI_TESForm, RTTI_TESNPC, 0);
	if (!act) return false;
	TransformNPC* tr_act = NULL;
	for each (TransformNPC* var in *npc_list) {
		if (var->ESPID == actor->GetModIndex() && var->RefID == actor->refID) {
			tr_act = var;
			break;
		}
	}
	if (tr_act == NULL)  return false;   //Reference is unregistered
	tr_act->status = TransformStatus::StatusUnTransform;  //Enqueue the Reference for Untransforming 
	return true;
}