#include "DFCommonInternal.h"
#include <shms.h>
#include <mod-core.h>
#include <mod-maps.h>
#include <mod-creature40d.h>
#include "private/APIPrivate.h"
#include "DFMemInfo.h"
#include "DFProcess.h"

#include "modules/Creatures.h"
#include "modules/Maps.h"
#include "modules/Materials.h"
#include "modules/Position.h"
#include "modules/Translation.h"
#include "modules/Gui.h"

using namespace DFHack;

APIPrivate::APIPrivate()
{
    // init modules
    creatures = 0;
    maps = 0;
    position = 0;
    gui = 0;
    materials = 0;
    translation = 0;
}

APIPrivate::~APIPrivate()
{
    if(creatures) delete creatures;
    if(maps) delete maps;
    if(position) delete position;
    if(gui) delete gui;
    if(materials) delete materials;
    if(translation) delete translation;
}

bool APIPrivate::InitReadNames()
{
    name_firstname_offset = offset_descriptor->getOffset("name_firstname");
    name_nickname_offset = offset_descriptor->getOffset("name_nickname");
    name_words_offset = offset_descriptor->getOffset("name_words");
    return true;
}

void APIPrivate::readName(t_name & name, uint32_t address)
{
    g_pProcess->readSTLString(address + name_firstname_offset , name.first_name, 128);
    g_pProcess->readSTLString(address + name_nickname_offset , name.nickname, 128);
    g_pProcess->read(address + name_words_offset ,48, (uint8_t *) name.words);
}