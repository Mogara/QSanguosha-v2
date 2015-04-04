#include "yjcm2015.h"
#include "general.h"





YJCM2015Package::YJCM2015Package()
    : Package("YJCM2015")
{

    General *zhangyi = new General(this, "zhangyi", "shu", 5);
    General *liuchen = new General(this, "liuchen", "shu");
    General *xiahou = new General(this, "yj_xiahoushi", "shu", 3, false);
    General *caoxiu = new General(this, "caoxiu", "wei");
    General *guofeng = new General(this, "guotufengji", "qun", 3);
    General *caorui = new General(this, "caorui$", "wei", 3);
    General *zhongyao = new General(this, "zhongyao", "wei", 3);
    General *quanzong = new General(this, "quanzong", "wu");
    General *zhuzhi = new General(this, "zhuzhi", "wu");
    General *sunxiu = new General(this, "sunxiu", "wu", 3);
    General *gongsun = new General(this, "gongsunyuan", "qun");


}
ADD_PACKAGE(YJCM2015)