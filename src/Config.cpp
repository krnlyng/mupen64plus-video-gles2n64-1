/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Config_nogui.cpp                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "Config.h"
#include "gles2N64.h"
#include "RSP.h"
#include "Textures.h"
#include "OpenGL.h"

#include "Config.h"
#include "Common.h"

Config config;

struct Option
{
    const char* name;
    int*  data;
    const int   initial;
};


#define CONFIG_VERSION 2

Option configOptions[] =
{
    {"#gles2n64 Graphics Plugin for N64", NULL, 0},
    {"#by Orkin / glN64 developers and Adventus.", NULL, 0},

    {"configversion", &config.version, 0},
    {"", NULL, 0},

    {"#Window Settings:", NULL, 0},
//    {"windowxpos", &config.window.xpos, 0},
//    {"windowypos", &config.window.ypos, 0},
    {"windowwidth", &config.window.width, 2048},
    {"windowheight", &config.window.height, 1536},
//    {"windowrefwidth", &config.window.refwidth, 400},
//    {"windowrefheight", &config.window.refheight, 240},
    {"multisampling", &config.multiSampling, 0},
#ifdef VC
    {"autoresolution", &config.useScreenResolution, 1},
#endif
    {"", NULL, 0},

    {"#Framebuffer Settings:",NULL,0},
//    {"framebufferenable", &config.framebuffer.enable, 0},
    {"framebufferbilinear", &config.framebuffer.bilinear, 0},
    {"framebufferwidth", &config.framebuffer.width, 640},
    {"framebufferheight", &config.framebuffer.height, 480},
//    {"framebufferwidth", &config.framebuffer.width, 800},
//    {"framebufferheight", &config.framebuffer.height, 480},
    {"", NULL, 0},

    {"#VI Settings:", NULL, 0},
    {"videoforce", &config.video.force, 0},
    {"videowidth", &config.video.width, 320},
    {"videoheight", &config.video.height, 240},
    {"videostretch", &config.stretchVideo, 1},
    {"videorotate", &config.video.rotate, 3},
    {"", NULL, 0},

    {"#Render Settings:", NULL, 0},
    {"enablefog", &config.enableFog, 1},
    {"enableprimitive z", &config.enablePrimZ, 1},
    {"enablelighting", &config.enableLighting, 1},
    {"enablealpha test", &config.enableAlphaTest, 1},
    {"enableclipping", &config.enableClipping, 0},
    {"enableface culling", &config.enableFaceCulling, 1},
    {"enablenoise", &config.enableNoise, 0},
    {"", NULL, 0},

    {"#Texture Settings:", NULL, 0},
    {"texture2xSAI", &config.texture.sai2x, 0},
    {"textureforce bilinear", &config.texture.forceBilinear, 0},
    {"texturemax anisotropy", &config.texture.maxAnisotropy, 0},
    {"textureuse IA", &config.texture.useIA, 0},
    {"texturefast CRC", &config.texture.fastCRC, 1},
    {"texturepow2", &config.texture.pow2, 1},
    {"", NULL, 0},

    {"#Frame skip:", NULL, 0},
    {"autoframeskip", &config.autoFrameSkip, 1},
    {"targetFPS", &config.targetFPS, 20},
    {"framerenderrate", &config.frameRenderRate, 1},
    {"verticalsync", &config.verticalSync, 0},
    {"", NULL, 0},

    {"#Other Settings:", NULL, 0},
    {"updatemode", &config.updateMode, SCREEN_UPDATE_AT_VI_UPDATE },
	{"print FPS", &config.printFPS, 0},
    {"ignoreoffscreenrendering", &config.ignoreOffscreenRendering, 0},
    {"forcescreenclear", &config.forceBufferClear, 0},
    {"flipvertical", &config.screen.flipVertical, 0},
// paulscode: removed from pre-compile to a config option
//// (part of the Galaxy S Zelda crash-fix
    {"tribufferopt", &config.tribufferOpt, 1},
//
    {"", NULL, 0},

    {"#Hack Settings:", NULL, 0},
    {"hackbanjotooie", &config.hackBanjoTooie, 0},
    {"hackzelda", &config.hackZelda, 0},
    {"hackalpha", &config.hackAlpha, 0},
    {"hackz", &config.zHack, 0},

};

const int configOptionsSize = sizeof(configOptions) / sizeof(Option);

void Config_WriteConfig(const char *filename)
{
    config.version = CONFIG_VERSION;
    FILE* f = fopen(filename, "w");
    if (!f)
    {
        LOG(LOG_ERROR, "Could Not Open %s for writing\n", filename);
    }

    for(int i=0; i<configOptionsSize; i++)
    {
        Option *o = &configOptions[i];
        fprintf(f, o->name);
        if (o->data) fprintf(f,"=%i", *(o->data));
        fprintf(f, "\n");
    }


    fclose(f);
}

void Config_SetDefault()
{
    m64p_handle video_general_section;
    m64p_handle video_gles2n64_section;
    m64p_error err = ConfigOpenSection("Video-General", &video_general_section);
    ConfigOpenSection("Video-gles2n64", &video_gles2n64_section);

    for(int i=0; i < configOptionsSize; i++)
    {
        Option *o = &configOptions[i];
        if(strcmp(o->name, "windowheight") == 0 && !err)
        {
            if(o->data) *(o->data) = ConfigGetParamInt(video_general_section, "ScreenHeight");
        }
        else if(strcmp(o->name, "windowwidth") == 0 && !err)
        {
            if(o->data) *(o->data) = ConfigGetParamInt(video_general_section, "ScreenWidth");
        }
        else if(strcmp(o->name, "videorotate") == 0 && !err)
        {
            if(o->data) *(o->data) = ConfigGetParamInt(video_general_section, "Rotate");
        }
        else
        {
            if (o->data)
            {
                ConfigSetDefaultInt(video_gles2n64_section, o->name, o->initial, "");
                *(o->data) = ConfigGetParamInt(video_gles2n64_section, o->name);
            }
        }
    }
    ConfigSaveSection("Video-gles2n64");
}

void Config_SetOption(char* line, char* val)
{
    for(int i=0; i< configOptionsSize; i++)
    {
        Option *o = &configOptions[i];
        if (strcasecmp(line, o->name) == 0)
        {
            if (o->data)
            {
                int v = atoi(val);
                *(o->data) = v;
                LOG(LOG_VERBOSE, "Config Option: %s = %i\n", o->name, v);
            }
            break;
        }
    }
}

void Config_LoadRomConfig(unsigned char* header)
{
    char line[4096];

    // get the name of the ROM
    for (int i=0; i<20; i++) config.romName[i] = header[0x20+i];
    config.romName[20] = '\0';
    while (config.romName[strlen(config.romName)-1] == ' ')
    {
        config.romName[strlen(config.romName)-1] = '\0';
    }

    switch(header[0x3e])
    {
        // PAL codes
        case 0x44:
        case 0x46:
        case 0x49:
        case 0x50:
        case 0x53:
        case 0x55:
        case 0x58:
        case 0x59:
            config.romPAL = true;
            break;

        // NTSC codes
        case 0x37:
        case 0x41:
        case 0x45:
        case 0x4a:
            config.romPAL = false;
            break;

        // Fallback for unknown codes
        default:
            config.romPAL = false;
    }

    LOG(LOG_MINIMAL, "Rom is %s\n", config.romPAL ? "PAL" : "NTSC");

    const char *filename = ConfigGetSharedDataFilepath("gles2n64rom.conf");
    FILE *f = fopen(filename,"r");
    if (!f)
    {
        LOG(LOG_MINIMAL, "Could not find %s Rom settings file, using global.\n", filename);
        return;
    }
    else
    {
        LOG(LOG_MINIMAL, "[gles2N64]: Searching %s Database for \"%s\" ROM\n", filename, config.romName);
        bool isRom = false;
        while (!feof(f))
        {
            fgets(line, 4096, f);
            if (line[0] == '\n') continue;

            if (strncmp(line,"rom name=", 9) == 0)
            {
                //Depending on the editor, end lines could be terminated by "LF" or "CRLF"
                char* lf = strchr(line, '\n'); //Line Feed
                char* cr = strchr(line, '\r'); //Carriage Return
                if (lf) *lf='\0';
                if (cr) *cr='\0';
                isRom = (strcasecmp(config.romName, line+9) == 0);
            }
            else
            {
                if (isRom)
                {
                    char* val = strchr(line, '=');
                    if (!val) continue;
                    *val++ = '\0';
                    Config_SetOption(line,val);
                    LOG(LOG_MINIMAL, "%s = %s", line, val);
                }
            }
        }
    }
	
    fclose(f);
}

void Config_LoadConfig()
{
    FILE *f;
    char line[4096];

    // default configuration
    Config_SetDefault();
#if 0
    // read configuration from shared data path
    const char *sharedDataFilename = ConfigGetSharedDataFilepath("gles2n64.conf");
    char filename[PATH_MAX]; // store the path to the config file to open
    if (sharedDataFilename == NULL)
    {
	// file does not exist in shared data path, therefore set the filename
	// to the user's config path, e.g. $HOME/.config/mupen64plus
    	const char *userConfigPath = ConfigGetUserConfigPath();
 	sprintf(filename, "%s/gles2n64.conf", userConfigPath);
    } else {
	strcpy(filename, sharedDataFilename);	
    }
	//if (filename == NULL) filename = "gles2n64.conf";
	
	f = fopen(filename, "r");
	if (!f)
	{
		LOG(LOG_MINIMAL, "[gles2N64]: Couldn't open config file '%s' for reading: %s\n", filename, strerror( errno ) );
		LOG(LOG_MINIMAL, "[gles2N64]: Attempting to write new Config \n");
		Config_WriteConfig(filename);
	}
	else
	{
		LOG(LOG_MINIMAL, "[gles2n64]: Loading Config from %s \n", filename);

		while (!feof( f ))
		{
		    char *val;
		    fgets( line, 4096, f );

		    if (line[0] == '#' || line[0] == '\n')
		        continue;

		    val = strchr( line, '=' );
		    if (!val) continue;

		    *val++ = '\0';

		     Config_SetOption(line,val);
		}

		if (config.version < CONFIG_VERSION)
		{
		    LOG(LOG_WARNING, "[gles2N64]: Wrong config version, rewriting config with defaults\n");
		    Config_SetDefault();
		    Config_WriteConfig(filename);
		}

		fclose(f);
	}
#endif
}

