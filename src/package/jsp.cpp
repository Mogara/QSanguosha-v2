#include "jsp.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "engine.h"
#include "clientplayer.h"


class Liangzhu : public TriggerSkill
{
public:
    Liangzhu() : TriggerSkill("liangzhu")
    {
        events << HpRecover;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Play;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        foreach (ServerPlayer *sun, room->getAllPlayers()) {
            if (TriggerSkill::triggerable(sun)) {
                QString choice = room->askForChoice(sun, objectName(), "draw+letdraw+dismiss", QVariant::fromValue(player));
                if (choice == "dismiss")
                    continue;

                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(sun, objectName());
                if (choice == "draw") {
                    sun->drawCards(1);
                    room->setPlayerMark(sun, "@liangzhu_draw", 1);
                } else if (choice == "letdraw") {
                    player->drawCards(2);
                    room->setPlayerMark(player, "@liangzhu_draw", 1);
                }
            }
        }
        return false;
    }
};

class Fanxiang : public TriggerSkill
{
public:
    Fanxiang() : TriggerSkill("fanxiang")
    {
        events << EventPhaseStart;
        frequency = Skill::Wake;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return (target != NULL && target->hasSkill(this) && target->getPhase() == Player::Start && target->getMark("@fanxiang") == 0);
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        bool flag = false;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->getMark("@liangzhu_draw") > 0 && p->isWounded()) {
                flag = true;
                break;
            }
        }
        if (flag) {
            room->broadcastSkillInvoke(objectName());

            room->doSuperLightbox("jsp_sunshangxiang", "fanxiang");

            //room->doLightbox("$fanxiangAnimate", 5000);
            room->notifySkillInvoked(player, objectName());
            room->setPlayerMark(player, "fanxiang", 1);
            if (room->changeMaxHpForAwakenSkill(player, 1) && player->getMark("fanxiang") > 0) {

                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->getMark("@liangzhu_draw") > 0)
                        room->setPlayerMark(p, "@liangzhu_draw", 0);
                }

                room->recover(player, RecoverStruct());
                room->handleAcquireDetachSkills(player, "-liangzhu|xiaoji");
            }
        }
        return false;
    }
};

class CihuaiVS : public ZeroCardViewAsSkill
{
public:
    CihuaiVS() : ZeroCardViewAsSkill("cihuai")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player) && player->getMark("@cihuai") > 0;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "slash" && player->getMark("@cihuai") > 0;
    }

    const Card *viewAs() const
    {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_" + objectName());
        return slash;
    }
};

class Cihuai : public TriggerSkill
{
public:
    Cihuai() : TriggerSkill("cihuai")
    {
        events << EventPhaseStart << CardsMoveOneTime << Death;
        view_as_skill = new CihuaiVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && (target->hasSkill(this) || target->getMark("ViewAsSkill_cihuaiEffect") > 0);
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::Play && !player->isKongcheng() && TriggerSkill::triggerable(player) && player->askForSkillInvoke(this, data)) {
                room->showAllCards(player);
                bool flag = true;
                foreach (const Card *card, player->getHandcards()) {
                    if (card->isKindOf("Slash")) {
                        flag = false;
                        break;
                    }
                }
                room->setPlayerMark(player, "cihuai_handcardnum", player->getHandcardNum());
                if (flag) {
                    room->broadcastSkillInvoke(objectName(), 2);
                    room->setPlayerMark(player, "@cihuai", 1);
                    room->setPlayerMark(player, "ViewAsSkill_cihuaiEffect", 1);
                } else
                    room->broadcastSkillInvoke(objectName(), 1);
            }
        } else if (triggerEvent == CardsMoveOneTime) {
            if (player->getMark("@cihuai") > 0 && player->getHandcardNum() != player->getMark("cihuai_handcardnum")) {
                room->setPlayerMark(player, "@cihuai", 0);
                room->setPlayerMark(player, "ViewAsSkill_cihuaiEffect", 0);
            }
        } else if (triggerEvent == Death) {
            room->setPlayerMark(player, "@cihuai", 0);
            room->setPlayerMark(player, "ViewAsSkill_cihuaiEffect", 0);
        }
        return false;
    }
};


class Nuzhan : public TriggerSkill
{
public:
    Nuzhan() : TriggerSkill("nuzhan")
    {
        events << PreCardUsed << CardUsed << ConfirmDamage;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (triggerEvent == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(use.from)) {
                if (use.card != NULL && use.card->isKindOf("Slash") && use.card->isVirtualCard() && use.card->subcardsLength() == 1 && Sanguosha->getCard(use.card->getSubcards().first())->isKindOf("TrickCard")) {
                    room->broadcastSkillInvoke(objectName(), 1);
                    room->addPlayerHistory(use.from, use.card->getClassName(), -1);
                    use.m_addHistory = false;
                    data = QVariant::fromValue(use);
                }
            }
        } else if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(use.from)) {
                if (use.card != NULL && use.card->isKindOf("Slash") && use.card->isVirtualCard() && use.card->subcardsLength() == 1 && Sanguosha->getCard(use.card->getSubcards().first())->isKindOf("EquipCard"))
                    use.card->setFlags("nuzhan_slash");
            }
        } else if (triggerEvent == ConfirmDamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card != NULL && damage.card->hasFlag("nuzhan_slash")) {
                if (damage.from != NULL)
                    room->sendCompulsoryTriggerLog(damage.from, objectName(), true);

                room->broadcastSkillInvoke(objectName(), 2);

                ++damage.damage;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

/*
class Nuzhan_tms : public TargetModSkill {
public:
Nuzhan_tms() : TargetModSkill("#nuzhan") {

}

int getResidueNum(const Player *from, const Card *card) const {
if (from->hasSkill("nuzhan")) {
if ((card->isVirtualCard() && card->subcardsLength() == 1 && Sanguosha->getCard(card->getSubcards().first())->isKindOf("TrickCard")) || card->hasFlag("Global_SlashAvailabilityChecker"))
return 1000;
}

return 0;
}
};
*/

class JspDanqi : public PhaseChangeSkill
{
public:
    JspDanqi() : PhaseChangeSkill("jspdanqi")
    {
        frequency = Wake;
    }

    bool triggerable(const ServerPlayer *guanyu, Room *room) const
    {
        return PhaseChangeSkill::triggerable(guanyu) && guanyu->getMark(objectName()) == 0 && guanyu->getPhase() == Player::Start && guanyu->getHandcardNum() > guanyu->getHp() && !lordIsLiubei(room);
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        Room *room = target->getRoom();
        room->broadcastSkillInvoke(objectName());
        //room->doLightbox("$JspdanqiAnimate");
        room->doSuperLightbox("jsp_guanyu", "jspdanqi");
        room->setPlayerMark(target, objectName(), 1);
        if (room->changeMaxHpForAwakenSkill(target) && target->getMark(objectName()) > 0)
            room->handleAcquireDetachSkills(target, "mashu|nuzhan");

        return false;
    }

private:
    static bool lordIsLiubei(const Room *room)
    {
        if (room->getLord() != NULL) {
            const ServerPlayer *const lord = room->getLord();
            if (lord->getGeneral() && lord->getGeneralName().contains("liubei"))
                return true;

            if (lord->getGeneral2() && lord->getGeneral2Name().contains("liubei"))
                return true;
        }

        return false;
    }
};

class Kunfen : public PhaseChangeSkill
{
public:
    Kunfen() : PhaseChangeSkill("kunfen")
    {

    }

    Frequency getFrequency(const Player *target) const
    {
        if (target != NULL) {
            return target->getMark("fengliang") > 0 ? NotFrequent : Compulsory;
        }

        return Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        if (invoke(target))
            effect(target);

        return false;
    }

private:
    bool invoke(ServerPlayer *target) const
    {
        return getFrequency(target) == Compulsory ? true : target->askForSkillInvoke(this);
    }

    void effect(ServerPlayer *target) const
    {
        Room *room = target->getRoom();

        if (getFrequency(target) == Compulsory)
            room->broadcastSkillInvoke(objectName(), 1);
        else
            room->broadcastSkillInvoke(objectName(), 2);

        room->loseHp(target);
        if (target->isAlive())
            target->drawCards(2, objectName());
    }
};

class Fengliang : public TriggerSkill
{
public:
    Fengliang() : TriggerSkill("fengliang")
    {
        frequency = Wake;
        events << EnterDying;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return TriggerSkill::triggerable(target) && target->getMark(objectName()) == 0;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        room->broadcastSkillInvoke(objectName());
        room->doSuperLightbox("jsp_jiangwei", objectName());

        room->addPlayerMark(player, objectName(), 1);
        if (room->changeMaxHpForAwakenSkill(player) && player->getMark(objectName()) > 0) {
            int recover = 2 - player->getHp();
            room->recover(player, RecoverStruct(NULL, NULL, recover));
            room->handleAcquireDetachSkills(player, "tiaoxin");

            if (player->hasSkill("kunfen", true))
                room->doNotify(player, QSanProtocol::S_COMMAND_UPDATE_SKILL, QVariant("kunfen"));
        }

        return false;
    }
};

class Chixin : public OneCardViewAsSkill
{  // Slash::isSpecificAssignee
public:
    Chixin() : OneCardViewAsSkill("chixin")
    {
        filter_pattern = ".|diamond";
        response_or_use = true;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player);
    }

    bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "jink" || pattern == "slash";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        //CardUseStruct::CardUseReason r = Sanguosha->currentRoomState()->getCurrentCardUseReason();
        QString p = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        Card *c = NULL;
        if (p == "jink")
            c = new Jink(Card::SuitToBeDecided, -1);
        else
            c = new Slash(Card::SuitToBeDecided, -1);

        if (c == NULL)
            return NULL;

        c->setSkillName(objectName());
        c->addSubcard(originalCard);
        return c;
    }
};

class ChixinTrigger : public TriggerSkill
{
public:
    ChixinTrigger() : TriggerSkill("chixin")
    {
        events << PreCardUsed << EventPhaseEnd;
        view_as_skill = new Chixin;
        global = true;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    int getPriority(TriggerEvent) const
    {
        return 8;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Slash") && player->getPhase() == Player::Play) {
                QSet<QString> s = player->property("chixin").toString().split("+").toSet();
                foreach(ServerPlayer *p, use.to)
                    s.insert(p->objectName());

                QStringList l = s.toList();
                room->setPlayerProperty(player, "chixin", l.join("+"));
            }
        } else if (player->getPhase() == Player::Play)
            room->setPlayerProperty(player, "chixin", QString());

        return false;
    }
};

class Suiren : public PhaseChangeSkill
{
public:
    Suiren() : PhaseChangeSkill("suiren")
    {
        frequency = Limited;
        limit_mark = "@suiren";
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Start && target->getMark("@suiren") > 0;
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        Room *room = target->getRoom();
        ServerPlayer *p = room->askForPlayerChosen(target, room->getAlivePlayers(), objectName(), "@suiren-draw", true);
        if (p == NULL)
            return false;

        room->broadcastSkillInvoke(objectName());
        room->doSuperLightbox("jsp_zhaoyun", "suiren");
        room->setPlayerMark(target, "@suiren", 0);
        
        room->handleAcquireDetachSkills(target, "-yicong");
        int maxhp = target->getMaxHp() + 1;
        room->setPlayerProperty(target, "maxhp", maxhp);
        room->recover(target, RecoverStruct());

        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, target->objectName(), p->objectName());
        p->drawCards(3, objectName());

        return false;
    }
};


JiqiaoCard::JiqiaoCard()
{
    target_fixed = true;
}

void JiqiaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    int n = subcardsLength() * 2;
    QList<int> card_ids = room->getNCards(n, false);
    CardMoveReason reason1(CardMoveReason::S_REASON_TURNOVER, source->objectName(), "jiqiao", QString());
    CardsMoveStruct move(card_ids, NULL, Player::PlaceTable, reason1);
    room->moveCardsAtomic(move, true);
    room->getThread()->delay();
    room->getThread()->delay();
    
    DummyCard get;
    DummyCard thro;

    foreach (int id, card_ids) {
        const Card *c = Sanguosha->getCard(id);
        if (c == NULL)
            continue;

        if (c->isKindOf("TrickCard"))
            get.addSubcard(c);
        else
            thro.addSubcard(c);
    }

    if (get.subcardsLength() > 0)
        source->obtainCard(&get);

    if (thro.subcardsLength() > 0) {
        CardMoveReason reason2(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "jiqiao", QString());
        room->throwCard(&thro, reason2, NULL);
    }
}

class JiqiaoVS : public ViewAsSkill
{
public:
    JiqiaoVS() : ViewAsSkill("jiqiao")
    {
        response_pattern = "@@jiqiao";
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return to_select->isKindOf("EquipCard") && !Self->isJilei(to_select);
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() == 0)
            return NULL;

        JiqiaoCard *jq = new JiqiaoCard;
        jq->addSubcards(cards);
        return jq;
    }
};

class Jiqiao : public PhaseChangeSkill
{
public:
    Jiqiao() : PhaseChangeSkill("jiqiao")
    {
        view_as_skill = new JiqiaoVS;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Play)
            return false;

        if (!player->canDiscard(player, "he"))
            return false;

        player->getRoom()->askForUseCard(player, "@@jiqiao", "@jiqiao", -1, Card::MethodDiscard);

        return false;
    }
};

class Linglong : public TriggerSkill
{
public:
    Linglong() : TriggerSkill("linglong")
    {
        frequency = Compulsory;
        events << CardAsked;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL && target->hasArmorEffect("eight_diagram");
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *wolong, QVariant &data) const
    {
        QString pattern = data.toStringList().first();

        if (pattern != "jink")
            return false;

        if (wolong->askForSkillInvoke("eight_diagram")) {
            JudgeStruct judge;
            judge.pattern = ".|red";
            judge.good = true;
            judge.reason = "eight_diagram";
            judge.who = wolong;

            room->judge(judge);

            if (judge.isGood()) {
                room->setEmotion(wolong, "armor/eight_diagram");
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName("eight_diagram");
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class LinglongMax : public MaxCardsSkill
{
public:
    LinglongMax() : MaxCardsSkill("#linglong-horse")
    {

    }

    int getExtra(const Player *target) const
    {
        if (target->hasSkill("linglong") && target->getDefensiveHorse() == NULL && target->getOffensiveHorse() == NULL)
            return 1;

        return 0;
    }
};

class LinglongTreasure : public TriggerSkill
{
public:
    LinglongTreasure() : TriggerSkill("#linglong-treasure")
    {
        events << GameStart << EventAcquireSkill << EventLoseSkill << CardsMoveOneTime;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventLoseSkill && data.toString() == "linglong") {
            room->handleAcquireDetachSkills(player, "-qicai", true);
            player->setMark("linglong_qicai", 0);
        } else if ((triggerEvent == EventAcquireSkill && data.toString() == "linglong") || (triggerEvent == GameStart && TriggerSkill::triggerable(player))) {
            if (player->getTreasure() == NULL) {
                room->notifySkillInvoked(player, objectName());
                room->handleAcquireDetachSkills(player, "qicai");
                player->setMark("linglong_qicai", 1);
            }
        } else if (triggerEvent == CardsMoveOneTime && player->isAlive() && player->hasSkill("linglong", true)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && move.from_places.contains(Player::PlaceEquip)) {
                if (player->getTreasure() == NULL && player->getMark("linglong_qicai") == 0) {
                    room->notifySkillInvoked(player, objectName());
                    room->handleAcquireDetachSkills(player, "qicai");
                    player->setMark("linglong_qicai", 1);
                }
            } else if (move.to == player && move.to_place == Player::PlaceEquip) {
                if (player->getTreasure() != NULL && player->getMark("linglong_qicai") == 1) {
                    room->handleAcquireDetachSkills(player, "-qicai", true);
                    player->setMark("linglong_qicai", 0);
                }
            }
        }
        return false;
    }
};



JSPPackage::JSPPackage()
    : Package("jiexian_sp")
{
    General *jsp_sunshangxiang = new General(this, "jsp_sunshangxiang", "shu", 3, false); // JSP 001
    jsp_sunshangxiang->addSkill(new Liangzhu);
    jsp_sunshangxiang->addSkill(new Fanxiang);

    General *jsp_machao = new General(this, "jsp_machao", "qun"); // JSP 002
    jsp_machao->addSkill(new Skill("zhuiji", Skill::Compulsory));
    jsp_machao->addSkill(new Cihuai);

    General *jsp_guanyu = new General(this, "jsp_guanyu", "wei"); // JSP 003
    jsp_guanyu->addSkill("wusheng");
    jsp_guanyu->addSkill(new JspDanqi);
    jsp_guanyu->addRelateSkill("nuzhan");

    General *jsp_jiangwei = new General(this, "jsp_jiangwei", "wei");
    jsp_jiangwei->addSkill(new Kunfen);
    jsp_jiangwei->addSkill(new Fengliang);

    General *jsp_zhaoyun = new General(this, "jsp_zhaoyun", "qun", 3);
    jsp_zhaoyun->addSkill(new ChixinTrigger);
    jsp_zhaoyun->addSkill(new Suiren);
    jsp_zhaoyun->addSkill("yicong");

    General *jsp_huangyy = new General(this, "jsp_huangyueying", "qun", 3, false);
    jsp_huangyy->addSkill(new Jiqiao);
    jsp_huangyy->addSkill(new Linglong);
    jsp_huangyy->addSkill(new LinglongTreasure);
    jsp_huangyy->addSkill(new LinglongMax);
    related_skills.insertMulti("linglong", "#linglong-horse");
    related_skills.insertMulti("linglong", "#linglong-treasure");

    skills << new Nuzhan;

    addMetaObject<JiqiaoCard>();
}

ADD_PACKAGE(JSP)