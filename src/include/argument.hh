#pragma once

#include "declare.hh"
#include "Flags.hh"

int number_argument(const char * argument, char* arg);
Flags::Bit entity_argument(const char * argument, char* arg);
int mult_argument(const char * argument, char* arg);
const char *one_argument(const char * argument, char* arg);

int number_argument(const String& argument, String& arg);
Flags::Bit entity_argument(const String& argument, String& arg);
int mult_argument(const String& argument, String& arg);
const char *one_argument(const String& argument, String& arg);
