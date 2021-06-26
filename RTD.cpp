/****************************************************************/
/*
Roll The Dice plugin for bzflag. Gives a chance of getting a random but powerfull flag to player, or death via explosion!

Jarrett Cigainero, 2021
	MIT License.
	Explicitly: feel free to use this plugin, including with your modifications, on any server.
*/
/****************************************************************/

#include "bzfsAPI.h"
#include "plugin_utils.h"

// Function for RTD
void rtdGive(int);
void checkExplode(int);

double playerLastCommandTime[255];
int rtdExplodePlayerTime[255];
bool rtdExplodePlayer[255];
int rtdExplodeMsgOldTime;
bool playerAlive[255];

class RTD : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
    virtual bool SlashCommand(int, bz_ApiString, bz_ApiString, bz_APIStringList*);
    virtual const char* Name ()
    {
        return "[R]oll [T]he [D]ice";
    }
    virtual void Init ( const char* config );

    virtual void Event ( bz_EventData *eventData )
    {
        switch (eventData->eventType){
            //Player Join
            case bz_ePlayerJoinEvent:{
                bz_PlayerJoinPartEventData_V1* joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
                playerLastCommandTime[joinData->playerID] = -16;    //When a player joins, make sure they can use RTD for the first time.
                rtdExplodePlayer[joinData->playerID] = 0;      //Please do not explode player who just joined. k? thx.
            }
            break;
            //Player Update
            case bz_ePlayerUpdateEvent:{
                //bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
                int i = 0;
                while (i < 254){
                    //Check to see who rolled explode.
                    if (rtdExplodePlayer[i] == 1){
                        bz_BasePlayerRecord *pr = bz_getPlayerByIndex(i);
                        if(pr->currentFlag == "SHield (+SH)"){
                            bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"What luck! %s has found a Shield flag and has stopped themselves from exploding!",bz_getPlayerCallsign (i));
                            bz_removePlayerFlag(i);         //RNGesus givith! RNGesus takeith!
                            rtdExplodePlayer[i] = 0;        //Stop the countdown! The bzflag GODs are looking after this player today!
                            rtdExplodeMsgOldTime = 0;
                            bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "flag_won.wav" );
                        }
                        //Countdown timer.
                        if(rtdExplodeMsgOldTime <= bz_getCurrentTime()){
                            int rtdExplodeCountdown = 10 - (bz_getCurrentTime() - rtdExplodePlayerTime[i]);
                            //Display seconds left and send a beep sound at every tick.
                            if (rtdExplodeCountdown > 0){
                                bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%d", rtdExplodeCountdown);
                                rtdExplodeMsgOldTime = bz_getCurrentTime() + 1;
                                bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "message_private.wav" );
                            }
                        }
                        //Make player who rolled explode ded.
                        if ((rtdExplodePlayerTime[i] + 10) <= bz_getCurrentTime() ){
                            bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "flag_lost.wav" );
                            bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s exploded.", bz_getPlayerCallsign (i));
                            bz_killPlayer (i, 0, i, "SR");  //ded
                            rtdExplodePlayer[i] = 0;        //Player exploded, don't explode them again.
                            rtdExplodeMsgOldTime = 0;
                        }
                    }
                    i++;
                }
            }
            break;
            case bz_ePlayerDieEvent:{
                bz_PlayerDieEventData_V1* dieData = (bz_PlayerDieEventData_V1*)eventData;
                playerAlive[dieData->playerID] = 0;
            }
            break;
            case bz_ePlayerSpawnEvent:{
                bz_PlayerSpawnEventData_V1* spawnData = (bz_PlayerSpawnEventData_V1*)eventData;
                playerAlive[spawnData->playerID] = 1;
            }
            break;

        default:
        break;
    }
        return;
    }
};

BZ_PLUGIN(RTD)

void RTD::Init ( const char* /*commandLine*/ )
{
    bz_debugMessage(4,"RTD plugin loaded");

    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerUpdateEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_ePlayerDieEvent);

    bz_registerCustomSlashCommand("rtd", this);
    bz_registerCustomSlashCommand("RTD", this);
    bz_registerCustomSlashCommand("squish", this);

    // init events here with Register();
}

bool RTD::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    bz_BasePlayerRecord *pr = bz_getPlayerByIndex(playerID);
    if (command == "rtd"){
        if (pr->team != eObservers){
            rtdGive(playerID);
        }
        else {
            bz_sendTextMessagef(BZ_SERVER,playerID,"You cannot RTD as observer.");
        }
        return true;
    }
    else if (command == "RTD"){
        if (pr->team != eObservers){
            rtdGive(playerID);
        }
        else {
            bz_sendTextMessagef(BZ_SERVER,playerID,"You cannot RTD as observer.");
        }
        return true;
    }
    else if (command == "squish"){
        if (pr->team != eObservers){
            checkExplode(playerID);
        }
        else {
            bz_sendTextMessagef(BZ_SERVER,playerID,"You cannot be squished as observer.");
        }
        return true;
    }
    return false;
}

// Give a random power flag, random bad flag, or just kill the player.
void rtdGive(int giveID){
    if(playerAlive[giveID]){
        if ((playerLastCommandTime[giveID] + 15) <= bz_getCurrentTime()){
            playerLastCommandTime[giveID] = bz_getCurrentTime();
            switch (rand() % 14) {
                case 0:{
                    checkExplode(giveID);
                } break;
                case 1:{
                    bz_givePlayerFlag(giveID, "US", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Useless Flag. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 2:{
                    bz_givePlayerFlag(giveID, "US", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Useless Flag. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 3:{
                    bz_givePlayerFlag(giveID, "GM", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Guided Missile. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 4:{
                    bz_givePlayerFlag(giveID, "WG", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Wings. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 5:{
                    bz_givePlayerFlag(giveID, "WG", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Wings. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 6:{
                    bz_givePlayerFlag(giveID, "SW", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Shock Wave. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 7:{
                    bz_givePlayerFlag(giveID, "BU", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Burrow. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 8:{
                    bz_givePlayerFlag(giveID, "L", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Laser. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 9:{
                    bz_givePlayerFlag(giveID, "SB", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Super Bullet. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 10:{
                    bz_givePlayerFlag(giveID, "MG", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Machine Gun. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 11:{
                    bz_givePlayerFlag(giveID, "ST", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Stealth. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 12:{
                    bz_givePlayerFlag(giveID, "ST", 1);
                    bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled Stealth. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
                } break;
                case 13:{
                    checkExplode(giveID);
                } break;
            }
        }
        else {
            int timeLeft = 15 - (bz_getCurrentTime() - playerLastCommandTime[giveID]);
            bz_sendTextMessagef(BZ_SERVER,giveID,"You must wait %d more seconds before Rolling The Dice again.", timeLeft);
        }
    }
    else{
        bz_sendTextMessagef(BZ_SERVER,giveID,"You must be alive to Roll The Dice.");
    }
}

void checkExplode(int giveID){
    bz_BasePlayerRecord *pr = bz_getPlayerByIndex(giveID);
    if(pr->currentFlag == "SHield (+SH)"){
        bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled EXPLODE! but they are holding the Shield flag. Lucky them! Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
        bz_removePlayerFlag(giveID);    //RNGesus givith! RNGesus takeith!
    }
    else {
        rtdExplodePlayer[giveID] = 1;
        rtdExplodePlayerTime[giveID] = bz_getCurrentTime();
        bz_sendTextMessagef(BZ_SERVER,BZ_ALLUSERS,"%s has rolled EXPLODE! ded in 10 seconds lol. Use /rtd to [R]oll [T]he [D]ice!",bz_getPlayerCallsign (giveID));
        bz_sendPlayCustomLocalSound (BZ_ALLUSERS, "spree4.wav" );
    }
}
// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
