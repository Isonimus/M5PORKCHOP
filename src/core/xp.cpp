// Porkchop RPG XP and Leveling System Implementation

#include "xp.h"
#include "../ui/display.h"

// Static member initialization
PorkXPData XP::data = {0};
SessionStats XP::session = {0};
Preferences XP::prefs;
bool XP::initialized = false;
void (*XP::levelUpCallback)(uint8_t, uint8_t) = nullptr;

// XP values for each event type
static const uint16_t XP_VALUES[] = {
    1,      // NETWORK_FOUND
    5,      // NETWORK_HIDDEN
    10,     // NETWORK_WPA3
    3,      // NETWORK_OPEN
    50,     // HANDSHAKE_CAPTURED
    75,     // PMKID_CAPTURED
    2,      // DEAUTH_SENT
    15,     // DEAUTH_SUCCESS
    2,      // WARHOG_LOGGED
    25,     // DISTANCE_KM
    1,      // BLE_BURST
    3,      // BLE_APPLE
    10,     // GPS_LOCK
    25,     // ML_ROGUE_DETECTED
    50,     // SESSION_30MIN
    100,    // SESSION_60MIN
    200,    // SESSION_120MIN
    20      // LOW_BATTERY_CAPTURE
};

// 40 rank titles - Phrack swine flavor
static const char* RANK_TITLES[] = {
    // Tier 1: The Beginning (1-5)
    "SCRIPT PIGGY",
    "MUD SNORTER",
    "PACKET PIGLET",
    "NOOB ROOTER",
    "SLOP BUCKET HACKER",
    // Tier 2: Getting Serious (6-10)
    "TRUFFLE SNIFFER",
    "BACON APPRENTICE",
    "CHANNEL HOPPER",
    "DEAUTH DABBLER",
    "HAM HANDED HACKER",
    // Tier 3: Intermediate (11-15)
    "HANDSHAKE HUNTER",
    "ROGUE ROOTER",
    "PROMISCUOUS PORKER",
    "WARDRIVE WANDERER",
    "PCAP COLLECTOR",
    // Tier 4: Advanced (16-20)
    "EAPOL EVANGELIST",
    "FRAME INJECTOR",
    "SNOUT ZERO DAY",
    "PORK PROTOCOL",
    "EVIL TWIN FARMER",
    // Tier 5: Expert (21-25)
    "SILICON SWINE",
    "CHAOS SAUSAGE",
    "BLACKHAT BOAR",
    "802.11 WARLORD",
    "ALPHA ROOTER",
    // Tier 6: Elite (26-30)
    "KERNEL BACON",
    "NATION STATE SWINE",
    "ZERO CLICK HOG",
    "PWNED PORK SUPREME",
    "SHADOW BROKER BOAR",
    // Tier 7: Legendary (31-35)
    "MYTHIC MUD DWELLER",
    "ETERNAL OINK",
    "VOID SNORTER",
    "QUANTUM PIGLET",
    "ASTRAL ROOTER",
    // Tier 8: Godtier (36-40)
    "ELDER HOG",
    "PRIME PORCINE",
    "THE GREAT BOAR",
    "OMEGA SWINE",
    "LEGENDARY PORKCHOP"
};
static const uint8_t MAX_LEVEL = 40;

// Achievement names
static const char* ACHIEVEMENT_NAMES[] = {
    "FIRST BLOOD",
    "CENTURION",
    "MARATHON PIG",
    "NIGHT OWL",
    "GHOST HUNTER",
    "APPLE FARMER",
    "WARDRIVER",
    "DEAUTH KING",
    "PMKID HUNTER",
    "WPA3 SPOTTER",
    "GPS MASTER",
    "TOUCH GRASS",
    "SILICON PSYCHO",
    "CLUTCH CAPTURE",
    "SPEED RUN",
    "CHAOS AGENT"
};

// Level up phrases
static const char* LEVELUP_PHRASES[] = {
    "snout grew stronger",
    "new truffle unlocked",
    "skill issue? not anymore",
    "gg ez level up",
    "evolution complete",
    "power level rising",
    "oink intensifies",
    "XP printer go brrr",
    "grinding them levels",
    "swine on the rise"
};
static const uint8_t LEVELUP_PHRASE_COUNT = sizeof(LEVELUP_PHRASES) / sizeof(LEVELUP_PHRASES[0]);

void XP::init() {
    if (initialized) return;
    
    load();
    startSession();
    initialized = true;
    
    Serial.printf("[XP] Initialized - LV%d %s (%lu XP)\n", 
                  getLevel(), getTitle(), data.totalXP);
}

void XP::load() {
    prefs.begin("porkxp", true);  // Read-only
    
    data.totalXP = prefs.getUInt("totalxp", 0);
    data.achievements = prefs.getUInt("achieve", 0);
    data.lifetimeNetworks = prefs.getUInt("networks", 0);
    data.lifetimeHS = prefs.getUInt("hs", 0);
    data.lifetimeDeauths = prefs.getUInt("deauths", 0);
    data.lifetimeDistance = prefs.getUInt("distance", 0);
    data.lifetimeBLE = prefs.getUInt("ble", 0);
    data.hiddenNetworks = prefs.getUInt("hidden", 0);
    data.wpa3Networks = prefs.getUInt("wpa3", 0);
    data.gpsNetworks = prefs.getUInt("gpsnet", 0);
    data.sessions = prefs.getUShort("sessions", 0);
    data.cachedLevel = calculateLevel(data.totalXP);
    
    prefs.end();
}

void XP::save() {
    prefs.begin("porkxp", false);  // Read-write
    
    prefs.putUInt("totalxp", data.totalXP);
    prefs.putUInt("achieve", data.achievements);
    prefs.putUInt("networks", data.lifetimeNetworks);
    prefs.putUInt("hs", data.lifetimeHS);
    prefs.putUInt("deauths", data.lifetimeDeauths);
    prefs.putUInt("distance", data.lifetimeDistance);
    prefs.putUInt("ble", data.lifetimeBLE);
    prefs.putUInt("hidden", data.hiddenNetworks);
    prefs.putUInt("wpa3", data.wpa3Networks);
    prefs.putUInt("gpsnet", data.gpsNetworks);
    prefs.putUShort("sessions", data.sessions);
    
    prefs.end();
    
    Serial.printf("[XP] Saved - LV%d (%lu XP)\n", getLevel(), data.totalXP);
}

void XP::startSession() {
    memset(&session, 0, sizeof(session));
    session.startTime = millis();
    data.sessions++;
}

void XP::endSession() {
    save();
    Serial.printf("[XP] Session ended - +%lu XP this session\n", session.xp);
}

void XP::addXP(XPEvent event) {
    uint16_t amount = XP_VALUES[static_cast<uint8_t>(event)];
    
    // Track lifetime stats based on event type
    switch (event) {
        case XPEvent::NETWORK_FOUND:
        case XPEvent::NETWORK_OPEN:
            data.lifetimeNetworks++;
            session.networks++;
            break;
        case XPEvent::NETWORK_HIDDEN:
            data.lifetimeNetworks++;
            data.hiddenNetworks++;
            session.networks++;
            break;
        case XPEvent::NETWORK_WPA3:
            data.lifetimeNetworks++;
            data.wpa3Networks++;
            session.networks++;
            break;
        case XPEvent::HANDSHAKE_CAPTURED:
        case XPEvent::PMKID_CAPTURED:
            data.lifetimeHS++;
            session.handshakes++;
            break;
        case XPEvent::DEAUTH_SUCCESS:
            data.lifetimeDeauths++;
            session.deauths++;
            break;
        case XPEvent::DEAUTH_SENT:
            // Don't count sent, only success
            break;
        case XPEvent::WARHOG_LOGGED:
            data.gpsNetworks++;
            break;
        case XPEvent::BLE_BURST:
        case XPEvent::BLE_APPLE:
            data.lifetimeBLE++;
            session.blePackets++;
            break;
        case XPEvent::GPS_LOCK:
            session.gpsLockAwarded = true;
            break;
        default:
            break;
    }
    
    addXP(amount);
    checkAchievements();
}

void XP::addXP(uint16_t amount) {
    uint8_t oldLevel = data.cachedLevel;
    
    data.totalXP += amount;
    session.xp += amount;
    
    uint8_t newLevel = calculateLevel(data.totalXP);
    
    if (newLevel > oldLevel) {
        data.cachedLevel = newLevel;
        Serial.printf("[XP] LEVEL UP! %d -> %d (%s)\n", 
                      oldLevel, newLevel, getTitleForLevel(newLevel));
        
        if (levelUpCallback) {
            levelUpCallback(oldLevel, newLevel);
        }
    }
}

void XP::addDistance(uint32_t meters) {
    data.lifetimeDistance += meters;
    session.distanceM += meters;
    
    // Award XP per km (check if we crossed a km boundary)
    static uint32_t lastKmAwarded = 0;
    uint32_t currentKm = session.distanceM / 1000;
    
    if (currentKm > lastKmAwarded) {
        uint32_t newKms = currentKm - lastKmAwarded;
        for (uint32_t i = 0; i < newKms; i++) {
            addXP(XPEvent::DISTANCE_KM);
        }
        lastKmAwarded = currentKm;
    }
}

void XP::updateSessionTime() {
    uint32_t sessionMinutes = (millis() - session.startTime) / 60000;
    
    if (sessionMinutes >= 30 && !session.session30Awarded) {
        addXP(XPEvent::SESSION_30MIN);
        session.session30Awarded = true;
    }
    if (sessionMinutes >= 60 && !session.session60Awarded) {
        addXP(XPEvent::SESSION_60MIN);
        session.session60Awarded = true;
    }
    if (sessionMinutes >= 120 && !session.session120Awarded) {
        addXP(XPEvent::SESSION_120MIN);
        session.session120Awarded = true;
    }
}

uint8_t XP::calculateLevel(uint32_t xp) {
    // XP thresholds for each level
    // Designed for: L1-5 quick, L6-20 steady, L21-40 grind
    uint32_t thresholds[MAX_LEVEL] = {
        0,      // L1
        100,    // L2
        300,    // L3
        600,    // L4
        1000,   // L5
        1500,   // L6
        2300,   // L7
        3400,   // L8
        4800,   // L9
        6500,   // L10
        8500,   // L11
        11000,  // L12
        14000,  // L13
        17500,  // L14
        21500,  // L15
        26000,  // L16
        31000,  // L17
        36500,  // L18
        42500,  // L19
        49000,  // L20
        56000,  // L21
        64000,  // L22
        73000,  // L23
        83000,  // L24
        94000,  // L25
        106000, // L26
        120000, // L27
        136000, // L28
        154000, // L29
        174000, // L30
        197000, // L31
        223000, // L32
        252000, // L33
        284000, // L34
        319000, // L35
        359000, // L36
        404000, // L37
        454000, // L38
        514000, // L39
        600000  // L40
    };
    
    for (uint8_t i = MAX_LEVEL - 1; i > 0; i--) {
        if (xp >= thresholds[i]) {
            return i + 1;  // Levels are 1-indexed
        }
    }
    return 1;
}

uint32_t XP::getXPForLevel(uint8_t level) {
    if (level <= 1) return 0;
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    
    // Same thresholds as calculateLevel
    uint32_t thresholds[MAX_LEVEL] = {
        0, 100, 300, 600, 1000, 1500, 2300, 3400, 4800, 6500,
        8500, 11000, 14000, 17500, 21500, 26000, 31000, 36500, 42500, 49000,
        56000, 64000, 73000, 83000, 94000, 106000, 120000, 136000, 154000, 174000,
        197000, 223000, 252000, 284000, 319000, 359000, 404000, 454000, 514000, 600000
    };
    
    return thresholds[level - 1];
}

uint8_t XP::getLevel() {
    return data.cachedLevel > 0 ? data.cachedLevel : 1;
}

uint32_t XP::getTotalXP() {
    return data.totalXP;
}

uint32_t XP::getXPToNextLevel() {
    uint8_t level = getLevel();
    if (level >= MAX_LEVEL) return 0;
    
    return getXPForLevel(level + 1) - data.totalXP;
}

uint8_t XP::getProgress() {
    uint8_t level = getLevel();
    if (level >= MAX_LEVEL) return 100;
    
    uint32_t currentLevelXP = getXPForLevel(level);
    uint32_t nextLevelXP = getXPForLevel(level + 1);
    uint32_t levelRange = nextLevelXP - currentLevelXP;
    uint32_t progress = data.totalXP - currentLevelXP;
    
    if (levelRange == 0) return 100;
    return (progress * 100) / levelRange;
}

const char* XP::getTitle() {
    return getTitleForLevel(getLevel());
}

const char* XP::getTitleForLevel(uint8_t level) {
    if (level < 1) level = 1;
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    return RANK_TITLES[level - 1];
}

void XP::unlockAchievement(PorkAchievement ach) {
    if (hasAchievement(ach)) return;
    
    data.achievements |= ach;
    
    // Find achievement index for name lookup
    uint8_t idx = 0;
    uint32_t mask = 1;
    while (mask < (uint32_t)ach && idx < 16) {
        mask <<= 1;
        idx++;
    }
    
    Serial.printf("[XP] Achievement unlocked: %s\n", ACHIEVEMENT_NAMES[idx]);
    
    // Could trigger a toast here
}

bool XP::hasAchievement(PorkAchievement ach) {
    return (data.achievements & ach) != 0;
}

uint32_t XP::getAchievements() {
    return data.achievements;
}

const char* XP::getAchievementName(PorkAchievement ach) {
    uint8_t idx = 0;
    uint32_t mask = 1;
    while (mask < (uint32_t)ach && idx < 16) {
        mask <<= 1;
        idx++;
    }
    return ACHIEVEMENT_NAMES[idx];
}

void XP::checkAchievements() {
    // First handshake
    if (data.lifetimeHS >= 1 && !hasAchievement(ACH_FIRST_BLOOD)) {
        unlockAchievement(ACH_FIRST_BLOOD);
    }
    
    // 100 networks in session
    if (session.networks >= 100 && !hasAchievement(ACH_CENTURION)) {
        unlockAchievement(ACH_CENTURION);
    }
    
    // 10km walked (session)
    if (session.distanceM >= 10000 && !hasAchievement(ACH_MARATHON_PIG)) {
        unlockAchievement(ACH_MARATHON_PIG);
    }
    
    // 10 hidden networks
    if (data.hiddenNetworks >= 10 && !hasAchievement(ACH_GHOST_HUNTER)) {
        unlockAchievement(ACH_GHOST_HUNTER);
    }
    
    // 100 Apple BLE hits (check lifetimeBLE, rough proxy)
    if (data.lifetimeBLE >= 100 && !hasAchievement(ACH_APPLE_FARMER)) {
        unlockAchievement(ACH_APPLE_FARMER);
    }
    
    // 1000 lifetime networks
    if (data.lifetimeNetworks >= 1000 && !hasAchievement(ACH_WARDRIVER)) {
        unlockAchievement(ACH_WARDRIVER);
    }
    
    // 100 successful deauths
    if (data.lifetimeDeauths >= 100 && !hasAchievement(ACH_DEAUTH_KING)) {
        unlockAchievement(ACH_DEAUTH_KING);
    }
    
    // WPA3 network found
    if (data.wpa3Networks >= 1 && !hasAchievement(ACH_WPA3_SPOTTER)) {
        unlockAchievement(ACH_WPA3_SPOTTER);
    }
    
    // 100 GPS-tagged networks
    if (data.gpsNetworks >= 100 && !hasAchievement(ACH_GPS_MASTER)) {
        unlockAchievement(ACH_GPS_MASTER);
    }
    
    // 50km total walked
    if (data.lifetimeDistance >= 50000 && !hasAchievement(ACH_TOUCH_GRASS)) {
        unlockAchievement(ACH_TOUCH_GRASS);
    }
    
    // 5000 lifetime networks
    if (data.lifetimeNetworks >= 5000 && !hasAchievement(ACH_SILICON_PSYCHO)) {
        unlockAchievement(ACH_SILICON_PSYCHO);
    }
    
    // 1000 BLE packets
    if (data.lifetimeBLE >= 1000 && !hasAchievement(ACH_CHAOS_AGENT)) {
        unlockAchievement(ACH_CHAOS_AGENT);
    }
}

const PorkXPData& XP::getData() {
    return data;
}

const SessionStats& XP::getSession() {
    return session;
}

void XP::setLevelUpCallback(void (*callback)(uint8_t, uint8_t)) {
    levelUpCallback = callback;
}

void XP::drawBar(M5Canvas& canvas) {
    // Draw XP bar at bottom of main canvas (y=91, in the empty space)
    int barY = 91;
    
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_FG);
    canvas.setTextDatum(top_left);
    
    // Format: "LV## TITLE"
    char levelStr[8];
    snprintf(levelStr, sizeof(levelStr), "LV%d", getLevel());
    canvas.drawString(levelStr, 2, barY);
    
    // Title (truncate if needed)
    const char* title = getTitle();
    String titleStr = title;
    if (titleStr.length() > 14) {
        titleStr = titleStr.substring(0, 12) + "..";
    }
    canvas.drawString(titleStr, 28, barY);
    
    // Progress bar on right side
    int barX = 150;
    int barW = 60;
    int barH = 8;
    int progressBarY = barY + 2;
    
    // Draw bar outline
    canvas.drawRect(barX, progressBarY, barW, barH, COLOR_FG);
    
    // Fill progress
    uint8_t progress = getProgress();
    int fillW = (barW - 2) * progress / 100;
    if (fillW > 0) {
        canvas.fillRect(barX + 1, progressBarY + 1, fillW, barH - 2, COLOR_FG);
    }
    
    // Percentage on far right
    char pctStr[8];
    snprintf(pctStr, sizeof(pctStr), "%d%%", progress);
    canvas.setTextDatum(top_right);
    canvas.drawString(pctStr, DISPLAY_W - 2, barY);
}
