#ifndef MODULE_H
#define	MODULE_H

typedef void (*ModuleFunc)(int arg);

typedef struct {
  char name[10];
  ModuleFunc initialize;
  ModuleFunc shutdown;
  int arg;
} Module;

#define MODULE(name, initialize, shutdown, arg)  \
    Module _module##name                         \
        __attribute__((section(".modules"))) = { \
      #name,                                     \
      initialize,                                \
      shutdown,                                  \
      arg                                        \
    }

void ModuleInit();
void ModuleShutdown();

#endif	// MODULE_H
