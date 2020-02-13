#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#include "Hooks.h"
#include "Redirector.h"


#if OBLIVION
#include "obse/GameAPI.h"

/*	As of 0020, ExtractArgsEx() and ExtractFormatStringArgs() are no longer directly included in plugin builds.
	They are available instead through the OBSEScriptInterface.
	To make it easier to update plugins to account for this, the following can be used.
	It requires that g_scriptInterface is assigned correctly when the plugin is first loaded.
*/
#define ENABLE_EXTRACT_ARGS_MACROS 1	// #define this as 0 if you prefer not to use this

#if ENABLE_EXTRACT_ARGS_MACROS

OBSEScriptInterface * g_scriptInterface = NULL;	// make sure you assign to this
#define ExtractArgsEx(...) g_scriptInterface->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_scriptInterface->ExtractFormatStringArgs(__VA_ARGS__)

#endif

#else
#include "obse_editor/EditorAPI.h"
#endif

#include "obse/ParamInfos.h"
#include "obse/Script.h"
#include "obse/GameObjects.h"
#include <string>

IDebugLog		gLog("Transformation Framework.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;


/***************************
* Serialization routines
***************************/

std::string	g_strData;

static void ResetData(void)
{
	g_strData.clear();
}

static void ExamplePlugin_SaveCallback(void * reserved)
{
	// write out the string
	g_serialization->OpenRecord('STR ', 0);
	g_serialization->WriteRecordData(g_strData.c_str(), g_strData.length());

	// write out some other data
	g_serialization->WriteRecord('ASDF', 1234, "hello world", 11);
}

static void ExamplePlugin_LoadCallback(void * reserved)
{
	UInt32	type, version, length;

	ResetData();

	char	buf[512];

	while(g_serialization->GetNextRecordInfo(&type, &version, &length))
	{
		_MESSAGE("record %08X (%.4s) %08X %08X", type, &type, version, length);

		switch(type)
		{
			case 'STR ':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("got string %s", buf);

				g_strData = buf;
				break;

			case 'ASDF':
				g_serialization->ReadRecordData(buf, length);
				buf[length] = 0;

				_MESSAGE("ASDF chunk = %s", buf);
				break;
			default:
				_MESSAGE("Unknown chunk type $08X", type);
		}
	}
}

static void ExamplePlugin_PreloadCallback(void * reserved)
{
	_MESSAGE("Preload Callback start");
	ExamplePlugin_LoadCallback(reserved);
	_MESSAGE("Preload Callback finished");
}

static void ExamplePlugin_NewGameCallback(void * reserved)
{
	ResetData();
}

/**************************
* Command definitions
**************************/

/*************************
	Messaging API example
*************************/

OBSEMessagingInterface* g_msg;

void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_ExitGame:
		_MESSAGE("Plugin Example received ExitGame message");
		break;
	case OBSEMessagingInterface::kMessage_ExitToMainMenu:
		_MESSAGE("Plugin Example received ExitToMainMenu message");
		break;
	case OBSEMessagingInterface::kMessage_PostLoad:
		_MESSAGE("Plugin Example received PostLoad mesage");
		break;
	case OBSEMessagingInterface::kMessage_LoadGame:
	case OBSEMessagingInterface::kMessage_SaveGame:
		_MESSAGE("Plugin Example received save/load message with file path %s", msg->data);
		break;
	case OBSEMessagingInterface::kMessage_PreLoadGame:
		_MESSAGE("Plugin Example received pre-loadgame message with file path %s", msg->data);
		break;
	case OBSEMessagingInterface::kMessage_ExitGame_Console:
		_MESSAGE("Plugin Example received quit game from console message");
		break;
	default:
		break;
	}
}

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Transformation Framework and other additions";
	info->version = 1;

	// version checks
	if(!obse->isEditor)
	{
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

#if OBLIVION
		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}
#endif

		g_serialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
		if(!g_serialization)
		{
			_ERROR("serialization interface not found");
			return false;
		}

		if(g_serialization->version < OBSESerializationInterface::kVersion)
		{
			_ERROR("incorrect serialization version found (got %08X need %08X)", g_serialization->version, OBSESerializationInterface::kVersion);
			return false;
		}

		g_arrayIntfc = (OBSEArrayVarInterface*)obse->QueryInterface(kInterface_ArrayVar);
		if (!g_arrayIntfc)
		{
			_ERROR("Array interface not found");
			return false;
		}

		g_scriptIntfc = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);		
	}
	else
	{
		// no version checks needed for editor
	}

	// version checks pass

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	_MESSAGE("load");

	g_pluginHandle = obse->GetPluginHandle();

	/***************************************************************************
	 *	
	 *	READ THIS!
	 *	-
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the OBSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and may not load in future versions of OBSE. See
	 *	obse_readme.txt for more information.
	 *	
	 **************************************************************************/
	// register commands
	//obse->SetOpcodeBase(0x2000);

	// set up serialization callbacks when running in the runtime
	if (!obse->isEditor)
	{
		InitHooks();
		CreareRedirections();
		//InstallRedirection
		// NOTE: SERIALIZATION DOES NOT WORK USING THE DEFAULT OPCODE BASE IN RELEASE BUILDS OF OBSE
		// it works in debug builds
		g_serialization->SetSaveCallback(g_pluginHandle, ExamplePlugin_SaveCallback);
		g_serialization->SetLoadCallback(g_pluginHandle, ExamplePlugin_LoadCallback);
		g_serialization->SetNewGameCallback(g_pluginHandle, ExamplePlugin_NewGameCallback);
#if 0	// enable below to test Preload callback, don't use unless you actually need it
		g_serialization->SetPreloadCallback(g_pluginHandle, ExamplePlugin_PreloadCallback);
#endif
	}
	return true;
}

};
