#ifndef _CLIENT_H
#define _CLIENT_H

#include "card.h"
#include "skill.h"
#include "room-state.h"
#include "protocol.h"

class Recorder;
class Replayer;
class QTextDocument;
class ClientSocket;

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Client::Status status READ getStatus WRITE setStatus)

    Q_ENUMS(Status)

public:
    enum Status
    {
        NotActive = 0x00,
        Responding = 0x01,
        Playing = 0x02,
        Discarding = 0x03,
        Exchanging = 0x04,
        ExecDialog = 0x05,
        AskForSkillInvoke = 0x06,
        AskForAG = 0x07,
        AskForPlayerChoose = 0x08,
        AskForYiji = 0x09,
        AskForGuanxing = 0x0A,
        AskForGongxin = 0x0B,
        AskForShowOrPindian = 0x0C,
        AskForGeneralTaken = 0x0D,
        AskForArrangement = 0x0E,

        RespondingUse = 0x11,
        RespondingForDiscard = 0x21,
        RespondingNonTrigger = 0x31,

        ClientStatusBasicMask = 0x0F
    };

    explicit Client(QObject *parent, const QString &filename = QString());
    ~Client();

    // cheat functions
    void requestCheatGetOneCard(int card_id);
    void requestCheatChangeGeneral(const QString &name, bool isSecondaryHero = false);
    void requestCheatKill(const QString &killer, const QString &victim);
    void requestCheatDamage(const QString &source, const QString &target, DamageStruct::Nature nature, int points);
    void requestCheatRevive(const QString &name);
    void requestCheatRunScript(const QString &script);

    // other client requests
    void requestSurrender();

    void disconnectFromHost();
    void replyToServer(QSanProtocol::CommandType command, const QVariant &arg = QVariant());
    void requestServer(QSanProtocol::CommandType command, const QVariant &arg = QVariant());
    void notifyServer(QSanProtocol::CommandType command, const QVariant &arg = QVariant());
    void onPlayerResponseCard(const Card *card, const QList<const Player *> &targets = QList<const Player *>());
    void setStatus(Status status);
    Status getStatus() const;
    int alivePlayerCount() const;
    void onPlayerInvokeSkill(bool invoke);
    void onPlayerDiscardCards(const Card *card);
    void onPlayerReplyYiji(const Card *card, const Player *to);
    void onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards);
    void onPlayerAssignRole(const QList<QString> &names, const QList<QString> &roles);
    QList<const ClientPlayer *> getPlayers() const;
    inline int getPlayerCount() const
    {
        return player_count;
    }
    void speakToServer(const QString &text);
    ClientPlayer *getPlayer(const QString &name);
    bool save(const QString &filename) const;
    QList<QByteArray> getRecords() const;
    QString getReplayPath() const;
    Replayer *getReplayer() const;
    QString getPlayerName(const QString &str);
    QString getSkillNameToInvoke() const;
    QString getSkillNameToInvokeData() const;

    QTextDocument *getLinesDoc() const;
    QTextDocument *getPromptDoc() const;

    typedef void (Client::*Callback) (const QVariant &);

    void checkVersion(const QVariant &server_version);
    void setup(const QVariant &setup_str);
    void networkDelayTest(const QVariant &);
    void addPlayer(const QVariant &player_info);
    void removePlayer(const QVariant &player_name);
    void startInXs(const QVariant &);
    void arrangeSeats(const QVariant &seats);
    void activate(const QVariant &playerId);
    void startGame(const QVariant &);
    void hpChange(const QVariant &change_str);
    void maxhpChange(const QVariant &change_str);
    void resetPiles(const QVariant &);
    void setPileNumber(const QVariant &pile_str);
    void synchronizeDiscardPile(const QVariant &discard_pile);
    void gameOver(const QVariant &);
    void loseCards(const QVariant &);
    void getCards(const QVariant &);
    void updateProperty(const QVariant &);
    void killPlayer(const QVariant &player_arg);
    void revivePlayer(const QVariant &player_arg);
    void warn(const QVariant &reason_json);
    void setMark(const QVariant &mark_str);
    void showCard(const QVariant &show_str);
    void log(const QVariant &log_str);
    void speak(const QVariant &speak_data);
    void addHistory(const QVariant &history);
    void moveFocus(const QVariant &focus);
    void setEmotion(const QVariant &set_str);
    void skillInvoked(const QVariant &invoke_str);
    void animate(const QVariant &animate_str);
    void cardLimitation(const QVariant &limit);
    void setNullification(const QVariant &str);
    void enableSurrender(const QVariant &enabled);
    void exchangeKnownCards(const QVariant &players);
    void setKnownCards(const QVariant &set_str);
    void viewGenerals(const QVariant &str);
    void setFixedDistance(const QVariant &set_str);
    void setAttackRangePair(const QVariant &set_arg);
    void updateStateItem(const QVariant &state_str);
    void setAvailableCards(const QVariant &pile);
    void setCardFlag(const QVariant &pattern_str);
    void updateCard(const QVariant &arg);
    void updateBossLevel(const QVariant &arg);

    void fillAG(const QVariant &cards_str);
    void takeAG(const QVariant &take_str);
    void clearAG(const QVariant &);

    //interactive server callbacks
    void askForCardOrUseCard(const QVariant &);
    void askForAG(const QVariant &);
    void askForSinglePeach(const QVariant &);
    void askForCardShow(const QVariant &);
    void askForSkillInvoke(const QVariant &);
    void askForChoice(const QVariant &);
    void askForDiscard(const QVariant &);
    void askForExchange(const QVariant &);
    void askForSuit(const QVariant &);
    void askForKingdom(const QVariant &);
    void askForNullification(const QVariant &);
    void askForPindian(const QVariant &);
    void askForCardChosen(const QVariant &);
    void askForPlayerChosen(const QVariant &);
    void askForGeneral(const QVariant &);
    void askForYiji(const QVariant &);
    void askForGuanxing(const QVariant &);
    void showAllCards(const QVariant &);
    void askForGongxin(const QVariant &);
    void askForAssign(const QVariant &); // Assign roles at the beginning of game
    void askForSurrender(const QVariant &);
    void askForLuckCard(const QVariant &);
    void handleGameEvent(const QVariant &);
    //3v3 & 1v1
    void askForOrder(const QVariant &);
    void askForRole3v3(const QVariant &);
    void askForDirection(const QVariant &);

    // 3v3 & 1v1 methods
    void fillGenerals(const QVariant &generals);
    void askForGeneral3v3(const QVariant &);
    void takeGeneral(const QVariant &take_str);
    void startArrange(const QVariant &to_arrange);

    void recoverGeneral(const QVariant &);
    void revealGeneral(const QVariant &);

    void attachSkill(const QVariant &skill);
    void updateSkill(const QVariant &);

    inline RoomState *getRoomState()
    {
        return &_m_roomState;
    }
    inline Card *getCard(int cardId) const
    {
        return _m_roomState.getCard(cardId);
    }

    inline void setCountdown(const QSanProtocol::Countdown &countdown)
    {
        m_mutexCountdown.lock();
        m_countdown = countdown;
        m_mutexCountdown.unlock();
    }

    inline QSanProtocol::Countdown getCountdown()
    {
        m_mutexCountdown.lock();
        QSanProtocol::Countdown countdown = m_countdown;
        m_mutexCountdown.unlock();
        return countdown;
    }

    inline QList<int> getAvailableCards() const
    {
        return available_cards;
    }

    // public fields
    bool m_isDiscardActionRefusable;
    bool m_canDiscardEquip;
    QString m_cardDiscardPattern;
    bool m_noNullificationThisTime;
    QString m_noNullificationTrickName;
    const ClientPlayer *m_respondingUseFixedTarget;
    int discard_num;
    int min_num;
    QString skill_name;
    QList<const Card *> discarded_list;
    QStringList players_to_choose;
    int m_bossLevel;

public slots:
    void signup();
    void onPlayerChooseGeneral(const QString &_name);
    void onPlayerMakeChoice();
    void onPlayerChooseCard(int card_id = -2);
    void onPlayerChooseAG(int card_id);
    void onPlayerChoosePlayer(const Player *player);
    void trust();
    void addRobot(int num);

    void onPlayerReplyGongxin(int card_id = -1);

protected:
    // operation countdown
    QSanProtocol::Countdown m_countdown;
    // sync objects
    QMutex m_mutexCountdown;
    Status status;
    int alive_count;
    int swap_pile;
    RoomState _m_roomState;

private:
    ClientSocket *socket;
    bool m_isGameOver;
    bool m_isDisconnected;
    QHash<QSanProtocol::CommandType, Callback> m_interactions;
    QHash<QSanProtocol::CommandType, Callback> m_callbacks;
    QList<const ClientPlayer *> players;
    int player_count;
    QStringList ban_packages;
    Recorder *recorder;
    Replayer *replayer;
    QTextDocument *lines_doc, *prompt_doc;
    int pile_num;
    QString skill_to_invoke;
    QString skill_to_invoke_data;
    QList<int> available_cards;

    unsigned int _m_lastServerSerial;

    void updatePileNum();
    QString setPromptList(const QStringList &text);
    QString _processCardPattern(const QString &pattern);
    void commandFormatWarning(const QString &str, const QRegExp &rx, const char *command);

    bool _loseSingleCard(int card_id, CardsMoveStruct move);
    bool _getSingleCard(int card_id, CardsMoveStruct move);

private slots:
    void processServerPacket(const QString &cmd);
    void processServerPacket(const char *cmd);
    bool processServerRequest(const QSanProtocol::Packet &packet);
    void notifyRoleChange(const QString &new_role);
    void onPlayerChooseSuit();
    void onPlayerChooseKingdom();
    void alertFocus();
    void onPlayerChooseOrder();
    void onPlayerChooseRole3v3();

signals:
    void version_checked(const QString &version_number, const QString &mod_name);
    void server_connected();
    void error_message(const QString &msg);
    void player_added(ClientPlayer *new_player);
    void player_removed(const QString &player_name);
    // choice signal
    void generals_got(const QStringList &generals);
    void kingdoms_got(const QStringList &kingdoms);
    void suits_got(const QStringList &suits);
    void options_got(const QString &skillName, const QStringList &options);
    void cards_got(const ClientPlayer *player, const QString &flags, const QString &reason, bool handcard_visible,
        Card::HandlingMethod method, QList<int> disabled_ids);
    void roles_got(const QString &scheme, const QStringList &roles);
    void directions_got();
    void orders_got(QSanProtocol::Game3v3ChooseOrderCommand reason);

    void seats_arranged(const QList<const ClientPlayer *> &seats);
    void hp_changed(const QString &who, int delta, DamageStruct::Nature nature, bool losthp);
    void maxhp_changed(const QString &who, int delta);
    void status_changed(Client::Status oldStatus, Client::Status newStatus);
    void avatars_hiden();
    void pile_reset();
    void player_killed(const QString &who);
    void player_revived(const QString &who);
    void card_shown(const QString &player_name, int card_id);
    void log_received(const QStringList &log_str);
    void guanxing(const QList<int> &card_ids, bool single_side);
    void gongxin(const QList<int> &card_ids, bool enable_heart, QList<int> enabled_ids);
    void focus_moved(const QStringList &focus, QSanProtocol::Countdown countdown);
    void emotion_set(const QString &target, const QString &emotion);
    void skill_invoked(const QString &who, const QString &skill_name);
    void skill_acquired(const ClientPlayer *player, const QString &skill_name);
    void animated(int name, const QStringList &args);
    void player_speak(const QString &who, const QString &text);
    void line_spoken(const QString &line);
    void card_used();

    void game_started();
    void game_over();
    void standoff();
    void event_received(const QVariant &);

    void move_cards_lost(int moveId, QList<CardsMoveStruct> moves);
    void move_cards_got(int moveId, QList<CardsMoveStruct> moves);

    void skill_attached(const QString &skill_name);
    void skill_detached(const QString &skill_name);
    void do_filter();

    void nullification_asked(bool asked);
    void surrender_enabled(bool enabled);

    void ag_filled(const QList<int> &card_ids, const QList<int> &disabled_ids);
    void ag_taken(ClientPlayer *taker, int card_id, bool move_cards);
    void ag_cleared();

    void generals_filled(const QStringList &general_names);
    void general_taken(const QString &who, const QString &name, const QString &rule);
    void general_asked();
    void arrange_started(const QString &to_arrange);
    void general_recovered(int index, const QString &name);
    void general_revealed(bool self, const QString &general);

    void role_state_changed(const QString &state_str);
    void generals_viewed(const QString &reason, const QStringList &names);

    void assign_asked();
    void start_in_xs();

    void skill_updated(const QString &skill_name);
};

extern Client *ClientInstance;

#endif

