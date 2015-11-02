#pragma once

int load_config(const char *filepath);

void configs_init(void);
void configs_cleanup(void);

int configs_int(const char *key);
float configs_float(const char *key);
const char* configs_string(const char *key) ;
