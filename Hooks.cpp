/* 
//OBSE doesn't initalize it's mainloop until all plugins are loaded.
//Would require some kind of deferred loading.
UInt32 mainloop_address_patch = 0x0040F19D; //This is the address of the OBSE mainloop entrypoint
UInt32 mainloop_address_hook = NULL;

UInt32 get_jump_dest(UInt32 addres) {
	UInt8* ptr = (UInt8*)addres;
	if (*ptr != 0xE9) return NULL;
	UInt32* distance_jump = (UInt32*)(addres + 1);
	return *distance_jump + addres + 1 + 4;
}

*/

#include "obse_common/SafeWrite.cpp"


static UInt32 mainloop_patch = 0x0040F197; 
static UInt32 mainloop_return = 0x0040F19D;
static UInt32 mainloop__table_address = 0x00b3f928;

static UInt32 porcoddio = 0;

void Mainloop(void) {
	porcoddio = *(UInt32*)mainloop__table_address; //NOTE this statement is PARAMOUNT to the correct function of Oblivion
	//TODO find a way to express this in pure inline asm
	_MESSAGE("Mainloop executed  %x\n", porcoddio);
}

__declspec(naked) void Mainloop_asm(void) {
	__asm {
		pushad
		call Mainloop
		popad
		mov edx , porcoddio
		jmp [mainloop_return]  //Let execute OBSE mainloop
	}
}


bool InitHooks(void) {
	WriteRelJump(mainloop_patch, (UInt32) &Mainloop_asm);
	_MESSAGE("Hook mainloop at %x", mainloop_patch);
	return true;
}