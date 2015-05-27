#include "tw.h"
#include "general.h"
#include "skill.h"
#include "util.h"

class Yinqin : public PhaseChangeSkill
{
public:
    Yinqin() : PhaseChangeSkill("yinqin")
    {

    }

    bool triggerable(const ServerPlayer *target) const
    {
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        Room *room = target->getRoom();
        QString kingdom = target->getKingdom() == "wei" ? "shu" : target->getKingdom() == "shu" ? "wei" : "wei+shu";
        if (target->askForSkillInvoke(this)) {
            kingdom = room->askForChoice(target, objectName(), kingdom);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(target, objectName());
            room->setPlayerProperty(target, "kingdom", kingdom);
        }

        return false;
    }
};

class TWBaobian : public TriggerSkill
{
public:
    TWBaobian() : TriggerSkill("twbaobian")
    {
        events << DamageCaused;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card != NULL && (damage.card->isKindOf("Slash") || damage.card->isKindOf("Duel")) && !damage.chain && !damage.transfer && damage.by_user) {
            if (damage.to->getKingdom() == player->getKingdom()) {
                if (player->askForSkillInvoke(this, data)) {
                    if (damage.to->getHandcardNum() < damage.to->getMaxHp()) {
                        room->broadcastSkillInvoke(objectName(), 1);
                        int n = damage.to->getMaxHp() - damage.to->getHandcardNum();
                        room->drawCards(damage.to, n, objectName());
                    }
                    return true;
                }
            } else if (damage.to->getHandcardNum() > qMax(damage.to->getHp(), 0) && player->canDiscard(damage.to, "h")) {
                // Seems it is no need to use FakeMoveSkill & Room::askForCardChosen, so we ignore it.
                // If PlayerCardBox has changed for Room::askForCardChosen, please tell me, I will soon fix this.
                if (player->askForSkillInvoke(this, data)) {
                    room->broadcastSkillInvoke(objectName(), 2);
                    QList<int> hc = damage.to->handCards();
                    qShuffle(hc);
                    int n = damage.to->getHandcardNum() - qMax(damage.to->getHp(), 0);
                    QList<int> to_discard = hc.mid(0, n - 1);
                    DummyCard dc(to_discard);
                    room->throwCard(&dc, damage.to, player);
                }
            }
        }

        return false;
    }
};

class Tijin : public TriggerSkill
{
public:
    Tijin() : TriggerSkill("tijin")
    {
        events << TargetSpecifying << CardFinished;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (triggerEvent == TargetSpecifying) {
            if (use.from != NULL && use.card != NULL && use.card->isKindOf("Slash") && use.to.length() == 1) {
                ServerPlayer *zumao = room->findPlayerBySkillName(objectName());
                if (!TriggerSkill::triggerable(zumao) || use.from == zumao || !use.from->inMyAttackRange(zumao))
                    return false;

                if (!use.from->tag.value("tijin").canConvert(QVariant::Map))
                    use.from->tag["tijin"] = QVariantMap();

                QVariantMap tijin_map = use.from->tag.value("tijin").toMap();
                if (tijin_map.contains(use.card->toString())) {
                    tijin_map.remove(use.card->toString());
                    use.from->tag["tijin"] = tijin_map;
                }

                if (zumao->askForSkillInvoke(this, data)) {
                    room->broadcastSkillInvoke(objectName());
                    use.to.first()->removeQinggangTag(use.card);
                    use.to.clear();
                    use.to << zumao;

                    data = QVariant::fromValue(use);

                    tijin_map[use.card->toString()] = QVariant::fromValue(zumao);
                    use.from->tag["tijin"] = tijin_map;
                }
            }
        } else {
            if (use.from != NULL && use.card != NULL) {
                QVariantMap tijin_map = use.from->tag.value("tijin").toMap();
                if (tijin_map.contains(use.card->toString())) {
                    ServerPlayer *zumao = tijin_map.value(use.card->toString()).value<ServerPlayer *>();
                    if (zumao != NULL && zumao->isAlive() && zumao->canDiscard(use.from, "he")) {
                        int id = room->askForCardChosen(zumao, use.from, "he", objectName(), false, Card::MethodDiscard);
                        room->throwCard(id, use.from, zumao);
                    }
                }
                tijin_map.remove(use.card->toString());
                use.from->tag["tijin"] = tijin_map;
            }

        }

        return false;
    }
};

class Xiaolian : public TriggerSkill
{
public:
    Xiaolian() : TriggerSkill("xiaolian")
    {
        events << TargetConfirming << Damaged;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Slash") && use.to.length() == 1) {
                ServerPlayer *caoang = room->findPlayerBySkillName(objectName());
                if (!TriggerSkill::triggerable(caoang) || use.to.first() == caoang)
                    return false;

                if (!caoang->tag.value("xiaolian").canConvert(QVariant::Map))
                    caoang->tag["xiaolian"] = QVariantMap();

                QVariantMap xiaolian_map = caoang->tag.value("xiaolian").toMap();
                if (xiaolian_map.contains(use.card->toString())) {
                    xiaolian_map.remove(use.card->toString());
                    caoang->tag["xiaolian"] = xiaolian_map;
                }

                if (caoang->askForSkillInvoke(this, data)) {
                    room->broadcastSkillInvoke(objectName());
                    ServerPlayer *target = use.to.first();
                    use.to.first()->removeQinggangTag(use.card);
                    use.to.clear();
                    use.to << caoang;

                    data = QVariant::fromValue(use);

                    xiaolian_map[use.card->toString()] = QVariant::fromValue(target);
                    caoang->tag["xiaolian"] = xiaolian_map;
                }
            }
        } else {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card != NULL) {
                if (!player->tag.value("xiaolian").canConvert(QVariant::Map))
                    return false;

                QVariantMap xiaolian_map = player->tag.value("xiaolian").toMap();
                if (xiaolian_map.contains(damage.card->toString())) {
                    ServerPlayer *target = xiaolian_map.value(damage.card->toString()).value<ServerPlayer *>();
                    if (target != NULL && player->getCardCount(true) > 0) {
                        const Card *c = room->askForExchange(player, objectName(), 1, 1, true, "@xiaolian-put", true);
                        if (c != NULL)
                            target->addToPile("xlhorse", c);
                    }
                }
                xiaolian_map.remove(damage.card->toString());
                player->tag["xiaolian"] = xiaolian_map;
            }
        }

        return false;
    }
};

class XiaolianDist : public DistanceSkill
{
public:
    XiaolianDist() : DistanceSkill("#xiaolian-dist")
    {

    }

    int getCorrect(const Player *from, const Player *to) const
    {
        if (from != to)
            return to->getPile("xlhorse").length();

        return 0;
    }
};


TaiwanSPPackage::TaiwanSPPackage()
    : Package("Taiwan_sp")
{
    General *tw_caocao = new General(this, "tw_caocao$", "wei", 4, true, true); // TW SP 019
    tw_caocao->addSkill("nosjianxiong");
    tw_caocao->addSkill("hujia");

    General *tw_simayi = new General(this, "tw_simayi", "wei", 3, true, true);
    tw_simayi->addSkill("nosfankui");
    tw_simayi->addSkill("nosguicai");

    General *tw_xiahoudun = new General(this, "tw_xiahoudun", "wei", 4, true, true); // TW SP 025
    tw_xiahoudun->addSkill("nosganglie");

    General *tw_zhangliao = new General(this, "tw_zhangliao", "wei", 4, true, true); // TW SP 013
    tw_zhangliao->addSkill("nostuxi");

    General *tw_xuchu = new General(this, "tw_xuchu", "wei", 4, true, true);
    tw_xuchu->addSkill("nosluoyi");

    General *tw_guojia = new General(this, "tw_guojia", "wei", 3, true, true); // TW SP 015
    tw_guojia->addSkill("tiandu");
    tw_guojia->addSkill("nosyiji");

    General *tw_zhenji = new General(this, "tw_zhenji", "wei", 3, false, true); // TW SP 007
    tw_zhenji->addSkill("qingguo");
    tw_zhenji->addSkill("luoshen");

    General *tw_liubei = new General(this, "tw_liubei$", "shu", 4, true, true); // TW SP 017
    tw_liubei->addSkill("rende");
    tw_liubei->addSkill("jijiang");

    General *tw_guanyu = new General(this, "tw_guanyu", "shu", 4, true, true); // TW SP 018
    tw_guanyu->addSkill("wusheng");

    General *tw_zhangfei = new General(this, "tw_zhangfei", "shu", 4, true, true);
    tw_zhangfei->addSkill("paoxiao");

    General *tw_zhugeliang = new General(this, "tw_zhugeliang", "shu", 3, true, true); // TW SP 012
    tw_zhugeliang->addSkill("guanxing");
    tw_zhugeliang->addSkill("kongcheng");

    General *tw_zhaoyun = new General(this, "tw_zhaoyun", "shu", 4, true, true); // TW SP 006
    tw_zhaoyun->addSkill("longdan");

    General *tw_machao = new General(this, "tw_machao", "shu", 4, true, true); // TW SP 010
    tw_machao->addSkill("mashu");
    tw_machao->addSkill("nostieji");

    General *tw_huangyueying = new General(this, "tw_huangyueying", "shu", 3, false, true); // TW SP 011
    tw_huangyueying->addSkill("nosjizhi");
    tw_huangyueying->addSkill("nosqicai");

    General *tw_sunquan = new General(this, "tw_sunquan$", "wu", 4, true, true); // TW SP 021
    tw_sunquan->addSkill("zhiheng");
    tw_sunquan->addSkill("jiuyuan");

    General *tw_ganning = new General(this, "tw_ganning", "wu", 4, true, true); // TW SP 009
    tw_ganning->addSkill("qixi");

    General *tw_lvmeng = new General(this, "tw_lvmeng", "wu", 4, true, true);
    tw_lvmeng->addSkill("keji");

    General *tw_huanggai = new General(this, "tw_huanggai", "wu", 4, true, true); // TW SP 014
    tw_huanggai->addSkill("noskurou");

    General *tw_zhouyu = new General(this, "tw_zhouyu", "wu", 3, true, true);
    tw_zhouyu->addSkill("nosyingzi");
    tw_zhouyu->addSkill("nosfanjian");

    General *tw_daqiao = new General(this, "tw_daqiao", "wu", 3, false, true); // TW SP 005
    tw_daqiao->addSkill("nosguose");
    tw_daqiao->addSkill("liuli");

    General *tw_luxun = new General(this, "tw_luxun", "wu", 3, true, true); // TW SP 016
    tw_luxun->addSkill("nosqianxun");
    tw_luxun->addSkill("noslianying");

    General *tw_sunshangxiang = new General(this, "tw_sunshangxiang", "wu", 3, false, true); // TW SP 028
    tw_sunshangxiang->addSkill("jieyin");
    tw_sunshangxiang->addSkill("xiaoji");

    /*General *tw_huatuo = new General(this, "tw_huatuo", 3, true, true);
    tw_huatuo->addSkill("qingnang");
    tw_huatuo->addSkill("jijiu");*/

    General *tw_lvbu = new General(this, "tw_lvbu", "qun", 4, true, true); // TW SP 008
    tw_lvbu->addSkill("wushuang");

    General *tw_diaochan = new General(this, "tw_diaochan", "qun", 3, false, true); // TW SP 002
    tw_diaochan->addSkill("noslijian");
    tw_diaochan->addSkill("biyue");

    General *tw_xiaoqiao = new General(this, "tw_xiaoqiao", "wu", 3, false, true);
    tw_xiaoqiao->addSkill("tianxiang");
    tw_xiaoqiao->addSkill("hongyan");

    General *tw_yuanshu = new General(this, "tw_yuanshu", "qun", 4, true, true); // TW SP 004
    tw_yuanshu->addSkill("yongsi");
    tw_yuanshu->addSkill("weidi");
}

ADD_PACKAGE(TaiwanSP)

TaiwanYJCMPackage::TaiwanYJCMPackage()
: Package("Taiwan_yjcm")
{
    General *xiahb = new General(this, "twyj_xiahouba", "shu"); // TAI 001
    xiahb->addSkill(new Yinqin);
    xiahb->addSkill(new TWBaobian);

    General *zumao = new General(this, "twyj_zumao", "wu"); // TAI 002
    zumao->addSkill(new Tijin);

    General *caoang = new General(this, "twyj_caoang", "wei"); // TAI 003
    caoang->addSkill(new XiaolianDist);
    caoang->addSkill(new Xiaolian);
    related_skills.insertMulti("xiaolian", "#xiaolian-dist");
}

ADD_PACKAGE(TaiwanYJCM)