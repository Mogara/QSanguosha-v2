#ifndef _SERVER_H
#define _SERVER_H

class Room;
class QGroupBox;
class QLabel;
class QRadioButton;
class ServerSocket;
class ClientSocket;
class QtUpnpPortMapping;

#include "src/pch.h"

class Package;

class Select3v3GeneralDialog : public QDialog
{
    Q_OBJECT

public:
    Select3v3GeneralDialog(QDialog *parent);

private:
    QTabWidget *tab_widget;
    QSet<QString> ex_generals;

    void fillTabWidget();
    void fillListWidget(QListWidget *list, const Package *pack);

private slots:
    void save3v3Generals();
    void toggleCheck();
};

class BanlistDialog : public QDialog
{
    Q_OBJECT

public:
    BanlistDialog(QWidget *parent, bool view = false);

private:
    QList<QListWidget *>lists;
    QListWidget *list;
    int item;
    QStringList ban_list;
    QPushButton *add2nd;
    QMap<QString, QStringList> banned_items;
    QLineEdit *card_to_ban;

private slots:
    void addGeneral(const QString &name);
    void add2ndGeneral(const QString &name);
    void addPair(const QString &first, const QString &second);
    void doAdd2ndButton();
    void doAddButton();
    void doRemoveButton();
    void save();
    void saveAll();
    void switchTo(int item);
};

class ServerDialog : public QDialog
{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    int config();


private:
    QWidget *createBasicTab();
    QWidget *createPackageTab();
    QWidget *createAdvancedTab();
    QWidget *createMiscTab();
    QLayout *createButtonLayout();

    QGroupBox *createGameModeBox();
    QGroupBox *create1v1Box();
    QGroupBox *create3v3Box();
    QGroupBox *createXModeBox();

    QLineEdit *server_name_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *random_seat_checkbox;
    QCheckBox *enable_cheat_checkbox;
    QCheckBox *free_choose_checkbox;
    QCheckBox *free_assign_checkbox;
    QCheckBox *free_assign_self_checkbox;
    QLabel *pile_swapping_label;
    QSpinBox *pile_swapping_spinbox;
    QCheckBox *without_lordskill_checkbox;
    QCheckBox *sp_convert_checkbox;
    QSpinBox *maxchoice_spinbox;
    QLabel *lord_maxchoice_label;
    QSpinBox *lord_maxchoice_spinbox;
    QSpinBox *nonlord_maxchoice_spinbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *disable_chat_checkbox;
    QCheckBox *second_general_checkbox;
    QCheckBox *same_checkbox;
    QCheckBox *basara_checkbox;
    QCheckBox *hegemony_checkbox;
    QLabel *hegemony_maxchoice_label;
    QSpinBox *hegemony_maxchoice_spinbox;
    QLabel *hegemony_maxshown_label;
    QSpinBox *hegemony_maxshown_spinbox;
    QLabel *max_hp_label;
    QComboBox *max_hp_scheme_ComboBox;
    QLabel *scheme0_subtraction_label;
    QSpinBox *scheme0_subtraction_spinbox;
    QCheckBox *prevent_awaken_below3_checkbox;
    QComboBox *scenario_ComboBox;
    QComboBox *mini_scene_ComboBox;
    QPushButton *mini_scene_button;
    QPushButton *boss_mode_button;
    QLineEdit *address_edit;
    QLineEdit *port_edit;
    QSpinBox *game_start_spinbox;
    QSpinBox *nullification_spinbox;
    QCheckBox *minimize_dialog_checkbox;
    QCheckBox *ai_enable_checkbox;
    QCheckBox *ai_chat_checkbox;
    QSpinBox *ai_delay_spinbox;
    QCheckBox *ai_delay_altered_checkbox;
    QSpinBox *ai_delay_ad_spinbox;
    QCheckBox *surrender_at_death_checkbox;
    QCheckBox *luck_card_checkbox;
    QRadioButton *official_3v3_radiobutton;
    QComboBox *official_3v3_ComboBox;
    QComboBox *role_choose_ComboBox;
    QCheckBox *exclude_disaster_checkbox;
    QComboBox *official_1v1_ComboBox;
    QCheckBox *kof_using_extension_checkbox;
    QCheckBox *kof_card_extension_checkbox;
    QComboBox *role_choose_xmode_ComboBox;
    QCheckBox *disable_lua_checkbox;
    QCheckBox *checkBoxUpnp;
    QCheckBox *checkBoxAddToListServer;
    QPushButton *select_all_generals_button;
    QPushButton *deselect_all_generals_button;
    QPushButton *select_reverse_generals_button;
    QPushButton *select_all_cards_button;
    QPushButton *deselect_all_cards_button;
    QPushButton *select_reverse_cards_button;


    QButtonGroup *extension_group;
    QButtonGroup *mode_group;

    int accept_type; // -1 means console start while 1 means server start


    QList<QCheckBox *> m_generalPackages;
    QList<QCheckBox *> m_cardPackages;

private slots:
    void setMaxHpSchemeBox();

    void onConsoleButtonClicked();
    void onServerButtonClicked();
    void onDetectButtonClicked();
    void select3v3Generals();
    void edit1v1Banlist();
    void updateButtonEnablility(QAbstractButton *button);

    void doCustomAssign();
    void doBossModeCustomAssign();
    void setMiniCheckBox();

    void selectAllGenerals();
    void deselectAllGenerals();
    void selectReverseGenerals();

    void selectAllCards();
    void deselectAllCards();
    void selectReverseCards();
};

class BossModeCustomAssignDialog : public QDialog
{
    Q_OBJECT

public:
    BossModeCustomAssignDialog(QWidget *parent);
    void config();

private:
    QCheckBox *diff_revive_checkBox;
    QCheckBox *diff_recover_checkBox;
    QCheckBox *diff_draw_checkBox;
    QCheckBox *diff_reward_checkBox;
    QCheckBox *diff_incMaxHp_checkBox;
    QCheckBox *diff_decMaxHp_checkBox;

    QCheckBox *experience_checkBox;
    QCheckBox *optional_boss_checkBox;
    QCheckBox *endless_checkBox;

    QLabel *turn_limit_label;
    QSpinBox *turn_limit_spinBox;
};

class Scenario;
class ServerPlayer;

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent);

    friend class BanIpDialog;

    void broadcast(const QString &msg);
    bool listen();
    void daemonize();
    Room *createNewRoom();
    void signupPlayer(ServerPlayer *player);
    void checkUpnpAndListServer();

private:
    ServerSocket *server;
    Room *current;
    QSet<Room *> rooms;
    QHash<QString, ServerPlayer *> players;
    QSet<QString> addresses;
    QMultiHash<QString, QString> name2objname;
    bool created_successfully;
	int playerCount;

    QtUpnpPortMapping *upnpPortMapping;
    QNetworkAccessManager networkAccessManager;
    QNetworkReply *networkReply;
    bool serverListFirstReg;
    int tryTimes;

private slots:
    void processNewConnection(ClientSocket *socket);
    void processRequest(const char *request);
    void cleanup();
    void gameOver();

    void upnpFinished();
    void upnpTimeout();
    void listServerReply();
    void addToListServer();
    void sendListServerRequest();

signals:
    void server_message(const QString &);
    void newPlayer(ServerPlayer *player);
};

#endif

