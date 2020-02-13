#pragma once

struct Redirection {
	Redirection* next;
	void** original;
	void* redirection;
	wchar_t* name;
};


void AddRedirection(void** orig, void* redir, wchar_t* name);

void CreareRedirections(void);

void InstallRedirects(void);

