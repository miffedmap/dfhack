// This is a generic plugin that does nothing useful apart from acting as an example... of a plugin that does nothing :D

// some headers required for a plugin. Nothing special, just the basics.
#include "Core.h"
#include <Console.h>
#include <Export.h>
#include <PluginManager.h>

// DF data structure definition headers
#include "DataDefs.h"
#include "df/world.h"
#include "df/ui.h"
#include "df/item.h"
#include "df/creature_raw.h"
#include "df/caste_raw.h"
#include "df/body_part_raw.h"
#include "df/historical_figure.h"

#include "df/job_item.h"
#include "df/job_material_category.h"
#include "df/dfhack_material_category.h"
#include "df/matter_state.h"
#include "df/material_vec_ref.h"
#include "df/builtin_mats.h"

#include "df/descriptor_color.h"
#include "df/descriptor_pattern.h"
#include "df/descriptor_shape.h"

#include "df/physical_attribute_type.h"
#include "df/mental_attribute_type.h"
#include <df/color_modifier_raw.h>

//DFhack specific headers
#include "modules/Maps.h"
#include "modules/MapCache.h"
#include "modules/Materials.h"

//Needed for writing the protobuff stuff to a file.
#include <fstream>
#include <string>
#include <iomanip>

#include "RemoteFortressReader.pb.h"

#include "RemoteServer.h"

using namespace DFHack;
using namespace df::enums;
using namespace RemoteFortressReader;


// Here go all the command declarations...
// mostly to allow having the mandatory stuff on top of the file and commands on the bottom

static command_result GetMaterialList(color_ostream &stream, const EmptyMessage *in, MaterialList *out);




// A plugin must be able to return its name and version.
// The name string provided must correspond to the filename - skeleton.plug.so or skeleton.plug.dll in this case
DFHACK_PLUGIN("RemoteFortressReader");

// Mandatory init function. If you have some global state, create it here.
DFhackCExport command_result plugin_init ( color_ostream &out, std::vector <PluginCommand> &commands)
{
    //// Fill the command list with your commands.
    //commands.push_back(PluginCommand(
    //    "isoworldremote", "Dump north-west embark tile to text file for debug purposes.",
    //    isoWorldRemote, false, /* true means that the command can't be used from non-interactive user interface */
    //    // Extended help string. Used by CR_WRONG_USAGE and the help command:
    //    "  This command does nothing at all.\n"
    //    "Example:\n"
    //    "  isoworldremote\n"
    //    "    Does nothing.\n"
    //));
    return CR_OK;
}

DFhackCExport RPCService *plugin_rpcconnect(color_ostream &)
{
    RPCService *svc = new RPCService();
	svc->addFunction("GetMaterialList", GetMaterialList);
    return svc;
}

// This is called right before the plugin library is removed from memory.
DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    // You *MUST* kill all threads you created before this returns.
    // If everything fails, just return CR_FAILURE. Your plugin will be
    // in a zombie state, but things won't crash.
    return CR_OK;
}

df::matter_state GetState(df::material * mat, uint16_t temp = 10015)
{
	df::matter_state state = matter_state::Solid;
	if (temp >= mat->heat.melting_point)
		state = df::matter_state::Liquid;
	if (temp >= mat->heat.boiling_point)
		state = matter_state::Gas;
	return state;
}

static command_result GetMaterialList(color_ostream &stream, const EmptyMessage *in, MaterialList *out)
{
	if (!Core::getInstance().isWorldLoaded()) {
		//out->set_available(false);
		return CR_OK;
	}



	df::world_raws *raws = &df::global::world->raws;
	MaterialInfo mat;
	for (int i = 0; i < raws->inorganics.size(); i++)
	{
		mat.decode(0, i);
		MaterialDefinition *mat_def = out->add_material_list();
		mat_def->mutable_mat_pair()->set_mat_index(0);
		mat_def->mutable_mat_pair()->set_mat_type(i);
		mat_def->set_id(mat.getToken());
		mat_def->set_name(mat.toString()); //find the name at cave temperature;
		if (raws->inorganics[i]->material.state_color[GetState(&raws->inorganics[i]->material)] < raws->language.colors.size())
		{
			df::descriptor_color *color = raws->language.colors[raws->inorganics[i]->material.state_color[GetState(&raws->inorganics[i]->material)]];
			mat_def->mutable_state_color()->set_red(color->red);
			mat_def->mutable_state_color()->set_green(color->green);
			mat_def->mutable_state_color()->set_blue(color->blue);
		}
	}
	for (int i = 1; i < 19; i++)
	{
		int k = 0;
		if (i == 7)
			k = 1;// for coal.
		for (int j = 0; j <= k; j++)
		{
			mat.decode(i, j);
			MaterialDefinition *mat_def = out->add_material_list();
			mat_def->mutable_mat_pair()->set_mat_index(i);
			mat_def->mutable_mat_pair()->set_mat_type(j);
			mat_def->set_id(mat.getToken());
			mat_def->set_name(mat.toString()); //find the name at cave temperature;
			if (raws->mat_table.builtin[i]->state_color[GetState(raws->mat_table.builtin[i])] < raws->language.colors.size())
			{
				df::descriptor_color *color = raws->language.colors[raws->mat_table.builtin[i]->state_color[GetState(raws->mat_table.builtin[i])]];
				mat_def->mutable_state_color()->set_red(color->red);
				mat_def->mutable_state_color()->set_green(color->green);
				mat_def->mutable_state_color()->set_blue(color->blue);
			}
		}
	}
	for (int i = 0; i < raws->creatures.all.size(); i++)
	{
		df::creature_raw * creature = raws->creatures.all[i];
		for (int j = 0; j < creature->material.size(); j++)
		{
			mat.decode(j + 19, i);
			MaterialDefinition *mat_def = out->add_material_list();
			mat_def->mutable_mat_pair()->set_mat_index(j+19);
			mat_def->mutable_mat_pair()->set_mat_type(i);
			mat_def->set_id(mat.getToken());
			mat_def->set_name(mat.toString()); //find the name at cave temperature;
			if (creature->material[j]->state_color[GetState(creature->material[j])] < raws->language.colors.size())
			{
				df::descriptor_color *color = raws->language.colors[creature->material[j]->state_color[GetState(creature->material[j])]];
				mat_def->mutable_state_color()->set_red(color->red);
				mat_def->mutable_state_color()->set_green(color->green);
				mat_def->mutable_state_color()->set_blue(color->blue);
			}
		}
	}
	return CR_OK;
}