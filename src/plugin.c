/*
 * TeamSpeak 3 Voice Monitor Plugin
 * 基于 TeamSpeak 3 Plugin SDK
 * 捕获频道内用户语音活动状态并实时显示昵称
 */

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#pragma warning(disable : 4100) /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/public_errors_rare.h"
#include "ts3_functions.h"

#include "plugin.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) \
    { strncpy(dest, src, destSize - 1); (dest)[destSize - 1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 26

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_t to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result)
{
    int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
    *result    = (char*)malloc(outlen);
    if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
        *result = NULL;
        return -1;
    }
    return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name()
{
#ifdef _WIN32
    static char* result = NULL;
    if (!result) {
        const wchar_t* name = L"Voice Monitor";
        if (wcharToUtf8(name, &result) == -1) {
            result = "Voice Monitor";
        }
    }
    return result;
#else
    return "Voice Monitor";
#endif
}

/* Plugin version */
const char* ts3plugin_version()
{
    return "1.0.0";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion()
{
    return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author()
{
    return "Voice Monitor Dev";
}

/* Plugin description */
const char* ts3plugin_description()
{
    return "Monitor voice activity and display speaker nicknames in chat.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs)
{
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init()
{
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
    char pluginPath[PATH_BUFSIZE];

    printf("PLUGIN: Voice Monitor loaded\n");

    /* Example on how to query application, resources and configuration paths from client */
    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
    ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

    printf("PLUGIN: App path: %s\nResources path: %s\nConfig path: %s\nPlugin path: %s\n", appPath, resourcesPath, configPath, pluginPath);

    return 0; /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown()
{
    printf("PLUGIN: Voice Monitor unloaded\n");

    /*
     * Note:
     * If your plugin implements a settings dialog, it must be closed and deleted here, else the
     * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
     */

    /* Free pluginID if we registered it */
    if (pluginID) {
        free(pluginID);
        pluginID = NULL;
    }
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure()
{
    printf("PLUGIN: offersConfigure\n");
    return PLUGIN_OFFERS_NO_CONFIGURE; /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget)
{
    printf("PLUGIN: configure\n");
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id)
{
    const size_t sz = strlen(id) + 1;
    pluginID        = (char*)malloc(sz * sizeof(char));
    _strcpy(pluginID, sz, id); /* The id buffer will invalidate after exiting this function */
    printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword()
{
    return NULL; /* Not using commands */
}

/* Plugin processes console command. Return 0 if plugin handled the command, 1 if not handled. */
int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command)
{
    printf("PLUGIN: process command: '%s'\n", command);
    return 0; /* Plugin handled command */
}

/* Client changed current server connection handler */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID)
{
    printf("PLUGIN: currentServerConnectionChanged %llu (%llu)\n", (long long unsigned int)serverConnectionHandlerID, (long long unsigned int)ts3Functions.getCurrentServerConnectionHandlerID());
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle()
{
    return "Voice Monitor";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data)
{
    char* name;

    /* For demonstration purpose, display the name of the currently selected server, channel or client. */
    switch (type) {
        case PLUGIN_SERVER:
            if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &name) != ERROR_ok) {
                printf("Error getting virtual server name\n");
                return;
            }
            break;
        case PLUGIN_CHANNEL:
            if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_NAME, &name) != ERROR_ok) {
                printf("Error getting channel name\n");
                return;
            }
            break;
        case PLUGIN_CLIENT:
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &name) != ERROR_ok) {
                printf("Error getting client nickname\n");
                return;
            }
            break;
        default:
            printf("Invalid item type: %d\n", type);
            data = NULL; /* Ignore */
            return;
    }

    *data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));                   /* Must be allocated in the plugin! */
    snprintf(*data, INFODATA_BUFSIZE, "The nickname is [I]\"%s\"[/I]", name); /* bbCode is supported. HTML is not supported */
    ts3Functions.freeMemory(name);
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data)
{
    free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload()
{
    return 0; /* 1 = request autoloaded, 0 = do not request autoload */
}

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon)
{
    struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
    menuItem->type                  = type;
    menuItem->id                    = id;
    _strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
    _strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
    return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x)                                                                                                                                                                                                                                  \
    const size_t sz = x + 1;                                                                                                                                                                                                                                   \
    size_t       n  = 0;                                                                                                                                                                                                                                       \
    *menuItems      = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS                                                                                                                                                                                                                                       \
    (*menuItems)[n++] = NULL;                                                                                                                                                                                                                                  \
    assert(n == sz);

/*
 * Menu IDs for this plugin. Pass these IDs when creating a menuitem to the TS3 client. When the menu item is triggered,
 * ts3plugin_onMenuItemEvent will be called passing the menu ID of the triggered menu item.
 * These IDs are freely choosable by the plugin author. It's not really needed to use an enum, it just looks prettier.
 */
enum { MENU_ID_GLOBAL_1 = 1 };

/*
 * Initialize plugin menus.
 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
 */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon)
{
    /*
     * Create the menus
     * There are three types of menu items:
     * - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
     * - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
     * - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
     *
     * Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
     *
     * The menu text is required, max length is 128 characters
     *
     * The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
     * Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
     * plugin filename, without dll/so/dylib suffix
     * e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
     */

    BEGIN_CREATE_MENUS(1); /* IMPORTANT: Number of menu items must be correct! */
    CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Voice Monitor", "");
    END_CREATE_MENUS; /* Includes an assert checking if the number of menu items matched */

    /* Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
     * If unused, set menuIcon to NULL */
    *menuIcon = NULL;

    /* All memory allocated in this function will be automatically released by the TeamSpeak client later by calling ts3plugin_freeMemory */
}

/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description)
{
    struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
    _strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
    _strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
    return hotkey;
}

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x)                                                                                                                                                                                                                                \
    const size_t sz = x + 1;                                                                                                                                                                                                                                   \
    size_t       n  = 0;                                                                                                                                                                                                                                       \
    *hotkeys        = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS                                                                                                                                                                                                                                     \
    (*hotkeys)[n++] = NULL;                                                                                                                                                                                                                                    \
    assert(n == sz);

/*
 * Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
 * Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
 * This function is automatically called by the client after ts3plugin_init.
 */
void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys)
{
    /* Register hotkeys giving a keyword and a description.
     * The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
     * The description is shown in the clients hotkey dialog. */
    BEGIN_CREATE_HOTKEYS(1); /* Create 1 hotkey. Size must be correct for allocating memory. */
    CREATE_HOTKEY("voice_monitor_toggle", "Toggle Voice Monitor");
    END_CREATE_HOTKEYS;

    /* The client will call ts3plugin_freeMemory to release all allocated memory */
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
    if (newStatus == STATUS_CONNECTION_ESTABLISHED) { /* connection established and we have client and channels available */
        char*        s;
        char         msg[1024];
        anyID        myID;

        /* Print clientlib version */
        if (ts3Functions.getClientLibVersion(&s) == ERROR_ok) {
            printf("PLUGIN: Client lib version: %s\n", s);
            ts3Functions.freeMemory(s); /* Release string */
        } else {
            ts3Functions.logMessage("Error querying client lib version", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }

        /* Write plugin name and version to log */
        snprintf(msg, sizeof(msg), "Plugin %s, Version %s, Author: %s", ts3plugin_name(), ts3plugin_version(), ts3plugin_author());
        ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);

        /* Print own client ID and nickname on this server */
        if (ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
            ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }
        if (ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_NICKNAME, &s) != ERROR_ok) {
            ts3Functions.logMessage("Error querying client nickname", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }
        printf("PLUGIN: My client ID = %d, nickname = %s\n", myID, s);
        ts3Functions.freeMemory(s);
    }
}

void ts3plugin_onNewChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID) {}

void ts3plugin_onNewChannelCreatedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {}

void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {}

void ts3plugin_onChannelMoveEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 newChannelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {}

void ts3plugin_onUpdateChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {}

void ts3plugin_onUpdateClientEvent(uint64 serverConnectionHandlerID, anyID clientID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {}

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {}

void ts3plugin_onClientMoveSubscriptionEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility) {}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {}

void ts3plugin_onClientIDsEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, anyID clientID, const char* clientName) {}

void ts3plugin_onClientIDsFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onServerEditedEvent(uint64 serverConnectionHandlerID, anyID editerID, const char* editerName, const char* editerUniqueIdentifier) {}

void ts3plugin_onServerUpdatedEvent(uint64 serverConnectionHandlerID) {}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage)
{
    printf("PLUGIN: onServerErrorEvent %llu %s %d %s\n", (long long unsigned int)serverConnectionHandlerID, errorMessage, error, (returnCode ? returnCode : ""));
    if (returnCode) {
        /* A plugin could now check the returnCode with previously (when calling a function) remembered returnCodes and react accordingly */
        return 1;
    }
    return 0; /* If no plugin return code was used, the return value of this function is ignored */
}

void ts3plugin_onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage) {}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored)
{
    printf("PLUGIN: onTextMessageEvent %llu %d %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, targetMode, fromID, fromName, message, ffIgnored);

    /* Friend/Foe manager has ignored the message, so ignore here as well. */
    if (ffIgnored) {
        return 0; /* Client will ignore the message anyways, so return value here doesn't matter */
    }

    return 0; /* 0 = handle normally, 1 = client will ignore the text message */
}

/*
 * Called when a client starts or stops talking.
 * This is the correct way to monitor voice activity!
 * DO NOT use onEditPlaybackVoiceDataEvent for monitoring - it's called hundreds of times per second!
 */
void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID)
{
    /* Demonstrate usage of getClientDisplayName */
    char name[512];
    if (ts3Functions.getClientDisplayName(serverConnectionHandlerID, clientID, name, 512) == ERROR_ok) {
        if (status == STATUS_TALKING) {
            printf("--> %s starts talking\n", name);
            /* Display in chat */
            char msg[256];
            snprintf(msg, sizeof(msg), "[Voice Monitor] %s started talking", name);
            ts3Functions.printMessageToCurrentTab(msg);
        } else if (status == STATUS_NOT_TALKING) {
            printf("--> %s stops talking\n", name);
            /* Display in chat */
            char msg[256];
            snprintf(msg, sizeof(msg), "[Voice Monitor] %s stopped talking", name);
            ts3Functions.printMessageToCurrentTab(msg);
        }
    }
}

void ts3plugin_onConnectionInfoEvent(uint64 serverConnectionHandlerID, anyID clientID) {}

void ts3plugin_onServerConnectionInfoEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onChannelSubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onChannelUnsubscribeFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onChannelDescriptionUpdateEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onChannelPasswordChangedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onPlaybackShutdownCompleteEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onSoundDeviceListChangedEvent(const char* modeID, int playOrCap) {}

/*
 * Called when voice data is being played back. This callback is called hundreds of times per second!
 * DO NOT call printMessageToCurrentTab or other UI functions here - it will crash TeamSpeak!
 * Use onTalkStatusChangeEvent instead to monitor voice state changes.
 */
void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels)
{
    /* MUST be empty! Use onTalkStatusChangeEvent instead */
}

void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {}

void ts3plugin_onEditMixedPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {}

void ts3plugin_onEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, int* edited) {}

void ts3plugin_onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {}

void ts3plugin_onCustom3dRolloffCalculationWaveEvent(uint64 serverConnectionHandlerID, uint64 waveHandle, float distance, float* volume) {}

void ts3plugin_onUserLoggingMessageEvent(const char* logMessage, int logLevel, const char* logChannel, uint64 logID, const char* logTime, const char* completeLogString) {}

/* Clientlib rare */

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, uint64 time,
                                          const char* kickMessage)
{
}

int ts3plugin_onClientPokeEvent(uint64 serverConnectionHandlerID, anyID fromClientID, const char* pokerName, const char* pokerUniqueIdentity, const char* message, int ffIgnored)
{
    printf("PLUGIN onClientPokeEvent: %llu %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, fromClientID, pokerName, message, ffIgnored);

    /* Check if the Friend/Foe manager has already blocked this poke */
    if (ffIgnored) {
        return 0; /* Client will block anyways, doesn't matter what we return */
    }

    return 0; /* 0 = handle normally, 1 = client will ignore the poke */
}

void ts3plugin_onClientSelfVariableUpdateEvent(uint64 serverConnectionHandlerID, int flag, const char* oldValue, const char* newValue) {}

void ts3plugin_onFileListEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path, const char* name, uint64 size, uint64 datetime, int type, uint64 incompletesize, const char* returnCode) {}

void ts3plugin_onFileListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path) {}

void ts3plugin_onFileInfoEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* name, uint64 size, uint64 datetime) {}

void ts3plugin_onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char* name, int type, int iconID, int saveDB) {}

void ts3plugin_onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {}

void ts3plugin_onServerGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {}

void ts3plugin_onServerGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID) {}

void ts3plugin_onServerGroupClientListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, uint64 clientDatabaseID, const char* clientNameIdentifier, const char* clientUniqueID) {}

void ts3plugin_onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char* name, int type, int iconID, int saveDB) {}

void ts3plugin_onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onChannelGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {}

void ts3plugin_onChannelGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID) {}

void ts3plugin_onChannelPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {}

void ts3plugin_onChannelPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {}

void ts3plugin_onClientPermListEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {}

void ts3plugin_onClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID) {}

void ts3plugin_onChannelClientPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {}

void ts3plugin_onChannelClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID) {}

void ts3plugin_onClientChannelGroupChangedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, uint64 channelID, anyID clientID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {}

int ts3plugin_onServerPermissionErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, unsigned int failedPermissionID)
{
    return 0; /* See onServerErrorEvent for return code description */
}

void ts3plugin_onPermissionListGroupEndIDEvent(uint64 serverConnectionHandlerID, unsigned int groupEndID) {}

void ts3plugin_onPermissionListEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, const char* permissionName, const char* permissionDescription) {}

void ts3plugin_onPermissionListFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onPermissionOverviewEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, uint64 channelID, int overviewType, uint64 overviewID1, uint64 overviewID2, unsigned int permissionID, int permissionValue, int permissionNegated,
                                         int permissionSkip)
{
}

void ts3plugin_onPermissionOverviewFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName,
                                             const char* invokerUniqueIdentity)
{
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName,
                                               const char* invokerUniqueIdentity)
{
}

void ts3plugin_onClientNeededPermissionsEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, int permissionValue) {}

void ts3plugin_onClientNeededPermissionsFinishedEvent(uint64 serverConnectionHandlerID) {}

void ts3plugin_onFileTransferStatusEvent(anyID transferID, unsigned int status, const char* statusMessage, uint64 remotefileSize, uint64 serverConnectionHandlerID) {}

void ts3plugin_onClientChatClosedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {}

void ts3plugin_onClientChatComposingEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {}

void ts3plugin_onServerLogEvent(uint64 serverConnectionHandlerID, const char* logMsg) {}

void ts3plugin_onServerLogFinishedEvent(uint64 serverConnectionHandlerID, uint64 lastPos, uint64 fileSize) {}

void ts3plugin_onMessageListEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, uint64 timestamp, int flagRead) {}

void ts3plugin_onMessageGetEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, const char* message, uint64 timestamp) {}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {}

void ts3plugin_onClientNamefromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {}

void ts3plugin_onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {}

void ts3plugin_onComplainListEvent(uint64 serverConnectionHandlerID, uint64 targetClientDatabaseID, const char* targetClientNickName, uint64 fromClientDatabaseID, const char* fromClientNickName, const char* complainReason, uint64 timestamp) {}

void ts3plugin_onBanListEvent(uint64 serverConnectionHandlerID, uint64 banid, const char* ip, const char* name, const char* uid, const char* mytsid, uint64 creationTime, uint64 durationTime, const char* invokerName, uint64 invokercldbid,
                              const char* invokeruid, const char* reason, int numberOfEnforcements, const char* lastNickName)
{
}

void ts3plugin_onClientServerQueryLoginPasswordEvent(uint64 serverConnectionHandlerID, const char* loginPassword) {}

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity)
{
    printf("ON PLUGIN COMMAND: %s %s %d %s %s\n", pluginName, pluginCommand, invokerClientID, invokerName, invokerUniqueIdentity);
}

void ts3plugin_onIncomingClientQueryEvent(uint64 serverConnectionHandlerID, const char* commandText) {}

void ts3plugin_onServerTemporaryPasswordListEvent(uint64 serverConnectionHandlerID, const char* clientNickname, const char* uniqueClientIdentifier, const char* description, const char* password, uint64 timestampStart, uint64 timestampEnd,
                                                  uint64 targetChannelID, const char* targetChannelPW)
{
}

/* Client UI callbacks */

/*
 * Called from client when an avatar image has been downloaded to or deleted from cache.
 * This callback can be called spontaneously or in response to ts3Functions.getAvatar()
 */
void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* avatarPath)
{
    /* If avatarPath is NULL, the avatar got deleted */
    /* If not NULL, avatarPath contains the path to the avatar file in the TS3Client cache */
    if (avatarPath != NULL) {
        printf("onAvatarUpdated: %llu %d %s\n", (long long unsigned int)serverConnectionHandlerID, clientID, avatarPath);
    } else {
        printf("onAvatarUpdated: %llu %d - deleted\n", (long long unsigned int)serverConnectionHandlerID, clientID);
    }
}

/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 *
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
    printf("PLUGIN: onMenuItemEvent: serverConnectionHandlerID=%llu, type=%d, menuItemID=%d, selectedItemID=%llu\n", (long long unsigned int)serverConnectionHandlerID, type, menuItemID, (long long unsigned int)selectedItemID);
    switch (type) {
        case PLUGIN_MENU_TYPE_GLOBAL:
            /* Global menu item was triggered. selectedItemID is unused and set to zero. */
            switch (menuItemID) {
                case MENU_ID_GLOBAL_1:
                    /* Menu global 1 was triggered */
                    ts3Functions.printMessageToCurrentTab("[Voice Monitor] Plugin is active and monitoring voice activity.");
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* This function is called if a plugin hotkey was pressed. Omit if hotkeys are unused. */
void ts3plugin_onHotkeyEvent(const char* keyword)
{
    printf("PLUGIN: Hotkey event: %s\n", keyword);
    /* Identify the hotkey by keyword and handle here... */
    if (strcmp(keyword, "voice_monitor_toggle") == 0) {
        ts3Functions.printMessageToCurrentTab("[Voice Monitor] Toggled!");
    }
}

/* Called when recording a hotkey has finished after calling ts3Functions.requestHotkeyInputDialog */
void ts3plugin_onHotkeyRecordedEvent(const char* keyword, const char* key) {}

// This function receives your key Identifier you send to notifyKeyEvent and should return
// the friendly device name of the device this hotkey originates from. Used for display in UI.
const char* ts3plugin_keyDeviceName(const char* keyIdentifier)
{
    return NULL;
}

// This function translates the given key identifier to a friendly key name for display in the UI
const char* ts3plugin_displayKeyText(const char* keyIdentifier)
{
    return NULL;
}

// This is used internally as a prefix for hotkeys so we can store them without collisions.
// Should be unique across plugins.
const char* ts3plugin_keyPrefix()
{
    return "voicemonitor";
}

/* Called when client custom nickname changed */
void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {}
