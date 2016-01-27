#include "module.h"

#include <stdio.h>

extern Module const _modules_start;
extern Module const _modules_end;

void ModuleInit() {
  Module const *module;
  for (module = &_modules_start; module < &_modules_end; ++module) {
    if (module->initialize) {
      printf("Initializing %s\n", module->name);
      module->initialize(module->arg);
    }
  }
}

void ModuleShutdown() {
  Module const *module;
  for (module = &_modules_start; module < &_modules_end; ++module) {
    if (module->shutdown) {
      module->shutdown(module->arg);
    }
  }
}
