-- translation for YitianPackage

return {
	["yitian"] = "倚天包",
	["yitian_cards"] = "倚天卡牌包",

	["yitian_sword"] = "倚天剑",
	[":yitian_sword"] = "装备牌·武器\
		攻击范围：２\
		武器技能：每当你于回合外受到伤害结算完毕后，你可以使用一张【杀】；当你失去装备区里的【倚天剑】时，你可以对一名其他角色造成【倚天剑】造成的1点伤害。",
	["@YitianSword-lost"] = "你可以选择一名其他角色，对其造成【倚天剑】造成的1点伤害。",
	["yitian-lost"] = "倚天剑",
	["@YitianSword-slash"] = "您在回合外受到了伤害，您可以此时使用一张【杀】",

	-- 神曹操内测第三版
	["#yt_shencaocao"] = "超世之英杰",
	["yt_shencaocao"] = "魏武帝",
	["weiwudi_guixin"] = "归心",
	[":weiwudi_guixin"] = "结束阶段开始时，你可以选择一项：\
	  1. 改变一名其他角色的势力；\
	  2. 获得一个未加入游戏的武将牌上的主公技。",
	["weiwudi_guixin:yes"] = "改变一名其他角色的势力，或获得一个未加入游戏的武将牌上的主公技",
	["weiwudi_guixin:modify"] = "改变一名其他角色的势力",
	["weiwudi_guixin:obtain"] = "获得一个游戏外的武将牌上的主公技",

	-- 曹冲
	["#yt_caochong"] = "早夭的神童",
	["yt_caochong"] = "曹冲-倚天",
	["&yt_caochong"] = "曹冲",
	["ytchengxiang"] = "称象",
	[":ytchengxiang"] = "每当你受到一次伤害后，你可以弃置X张点数之和与造成伤害的牌的点数相等的牌并选择至多X名角色，这些角色：已受伤，回复1点体力；未受伤，摸两张牌。",
	["@ytchengxiang-card"] = "请弃置点数之和为 %arg 的卡牌以发动“称象”技能",
	["~ytchengxiang"] = "选择若干张牌→选择若干名角色→点击确定",
	["conghui"] = "聪慧",
	[":conghui"] = "<font color=\"blue\"><b>锁定技，</b></font>你跳过弃牌阶段。",
	["zaoyao"] = "早夭",
	[":zaoyao"] = "<font color=\"blue\"><b>锁定技，</b></font>结束阶段开始时，若你的手牌数大于13，你须弃置所有手牌并失去1点体力。",

	-- 张郃
	["#zhangjunyi"] = "计谋巧变",
	["zhangjunyi"] = "张儁乂",
	["jueji"] = "绝汲",
	[":jueji"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以与一名角色拼点：当你赢后，你获得对方的拼点牌。你可以重复此流程，直到你拼点没赢为止。",

	-- 陆抗
	["#lukang"] = "最后的良将",
	["lukang"] = "陆抗",
	["lukang_weiyan"] = "围堰",
	[":lukang_weiyan"] = "你可以将摸牌阶段视为出牌阶段，将出牌阶段视为摸牌阶段。",
	["kegou"] = "克构",
	[":kegou"] = "<font color=\"purple\"><b>觉醒技，</b></font>准备阶段开始时，若你是除主公外唯一的吴势力角色，你减少1点体力上限，获得技能“连营”。",
	["#KegouWake"] = "%from 是场上唯一的吴势力角色，满足“克构”的觉醒条件",
	["$KegouAnimate"] = "image=image/animate/kegou.png",
	["lukang_weiyan:draw2play"] = "请选择是否发动“围堰”将 摸牌阶段 视为 出牌阶段？",
	["lukang_weiyan:play2draw"] = "请选择是否发动“围堰”将 出牌阶段 视为 摸牌阶段？",

	-- 夏侯涓
	["#xiahoujuan"] = "樵采的美人",
	["xiahoujuan"] = "夏侯涓",
	["lianli"] = "连理",
	[":lianli"] = "准备阶段开始时，你可以选择一名男性角色，你与其进入连理状态直到你的下回合开始：其可以替你使用或打出【闪】，你可以替其使用或打出【杀】。",
	["tongxin"] = "同心",
	[":tongxin"] = "每当一名处于连理状态的角色受到1点伤害后，你可以令处于连理状态的角色各摸一张牌",
	["liqian"] = "离迁",
	[":liqian"] = "<font color=\"blue\"><b>锁定技，</b></font>若你处于连理状态，势力与连理对象的势力相同；当你处于未连理状态时，势力为魏",
	["lianli-slash"] = "连理（杀）",
	["lianlislash"] = "连理（杀）",
	["lianli-jink"] = "连理（闪）",
	["lianli-slash:slash"] = "请选择是否需要你的连理角色替你使用或打出【杀】？",
	["lianli-jink:jink"] = "请选择是否需要你的连理角色替你使用或打出【闪】？",
	["@lianli-slash"] = "请提供一张【杀】给你的连理对象",
	["@lianli-jink"] = "请提供一张【闪】给你的连理对象",
	[":lianli-slash"] = "与你处于连理状态的女性角色可以替你使用或打出【杀】",
	["@lianli-card"] = "请选择连理的目标",
	["~lianli"] = "选择一名男性角色→点击确定",
	["#LianliConnection"] = "%from 与 %to 结为连理",
	["@tied"] = "连理",

	-- 神司马
	["#jinxuandi"] = "祁山里的术士",
	["jinxuandi"] = "晋宣帝",
	["wuling"] = "五灵",
	[":wuling"] = "准备阶段开始时，你可选择一种五灵效果，该效果对场上所有角色生效\
	该效果直到你的下回合开始为止，你选择的五灵效果不可与上回合重复\
	[风]一名角色受到火属性伤害时，此伤害+1。\
	[雷]一名角色受到雷属性伤害时，此伤害+1。\
	[水]一名角色受【桃】效果影响回复的体力+1。\
	[火]一名角色受到的伤害均视为火焰伤害。\
	[土]一名角色受到的属性伤害大于1时，防止多余的伤害。",
	["#WulingWind"] = "%from 受到“<font color=\"yellow\"><b>五灵</b></font>”（风）的影响，火焰伤害从 %arg 上升到 %arg2",
	["#WulingThunder"] = "%from 受到“<font color=\"yellow\"><b>五灵</b></font>”（雷）的影响，雷电伤害从 %arg 上升到 %arg2",
	["#WulingFire"] = "%from 受到“<font color=\"yellow\"><b>五灵</b></font>”（火）的影响，伤害属性变为火焰属性",
	["#WulingWater"] = "%from 受到“<font color=\"yellow\"><b>五灵</b></font>”（水）的影响，此【<font color=\"yellow\"><b>桃</b></font>】额外回复1点体力。",
	["#WulingEarth"] = "%from 受到“<font color=\"yellow\"><b>五灵</b></font>”（土）的影响，伤害减少至 <font color=\"yellow\"><b>1</b></font> 点",
	["wuling:wind"] = "[风]一名角色受到火属性害时，此伤害+1",
	["wuling:thunder"] = "[雷]一名角色受到雷属性时，此伤害+1",
	["wuling:water"] = "[水]一名角色受【桃】效果影响回复的体力",
	["wuling:fire"] = "[火]一名角色受到的伤害均视为火焰伤害",
	["wuling:earth"] = "[土]一名角色受到的属性伤害大于1时，防止多余的伤害",
	["@wind"] = "五灵(风)",
	["@thunder"] = "五灵(雷)",
	["@fire"] = "五灵(火)",
	["@water"] = "五灵(水)",
	["@earth"] = "五灵(土)",

	-- 蔡琰
	["#caizhaoji"] = "乱世才女",
	["caizhaoji"] = "蔡昭姬",
	["guihan"] = "归汉",
	[":guihan"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以弃置两张花色相同的红色手牌并选择一名其他角色，与其交换位置。",
	["caizhaoji_hujia"] = "胡笳",
	[":caizhaoji_hujia"] = "结束阶段开始时，你可以进行一次判定：若结果为红色，你获得此判定牌，若如此做，你可以重复此流程。若你在一个阶段内发动“胡笳”判定过至少三次，你将武将牌翻面。",

	-- 陆逊
	["#luboyan"] = "玩火的少年",
	["luboyan"] = "陆伯言",
	["#luboyanf"] = "玩火的少女",
	["luboyanf"] = "陆伯言(女)",
	["shenjun"] = "神君",
	[":shenjun"] = "<font color=\"blue\"><b>锁定技，</b></font>游戏开始时，你选择自己的性别；准备阶段开始时，你须改变性别；每当你受到异性角色造成的非雷属性伤害时，你防止之。",
	["shaoying"] = "烧营",
	[":shaoying"] = "每当你对一名不处于连环状态的角色造成一次火焰伤害扣减体力前，你可选择一名其距离为1的一名角色，若如此做，此伤害结算完毕后，你进行一次判定：若结果为红色，你对其造成1点火属性伤害。",
	["#Shaoying"] = "%from 选择了“%arg”的目标 %to",
	["zonghuo"] = "纵火",
	[":zonghuo"] = "<font color=\"blue\"><b>锁定技，</b></font>你使用的【杀】视为火【杀】。",
	["#ShenjunChoose"] = "%from 选择了 %arg 作为初始性别",
	["#ShenjunProtect"] = "%to 的“%arg”被触发，异性(%from)的非雷属性伤害无效",
	["#ShenjunFlip"] = "%from 的“%arg”被触发，性别倒置",
	["#Zonghuo"] = "%from 的锁定技“%arg”被触发，【杀】变为火属性",

	-- 钟会
	["#zhongshiji"] = "狠毒的野心家",
	["zhongshiji"] = "钟士季",
	["gongmou"] = "共谋",
	["@conspiracy"] = "共谋",
	[":gongmou"] = "结束阶段开始时，你可以选择一名其他角色，若如此做，其于其下个摸牌阶段摸牌后，将X张手牌交给你（X为你手牌数与对方手牌数的较小值），然后你将X张手牌交给其。",
	["#GongmouExchange"] = "%from 发动了“%arg2”技能，与 %to 交换了 %arg 张手牌",

	-- 姜维
	["#jiangboyue"] = "赤胆的贤将",
	["jiangboyue"] = "姜伯约",
	["lexue"] = "乐学",
	[":lexue"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以令一名其他角色展示一张手牌：若此牌为基本牌或非延时类锦囊牌，你于当前回合内可以将与此牌同花色的牌当作该牌使用或打出；否则，你获得之。",
	["xunzhi"] = "殉志",
	[":xunzhi"] = "出牌阶段，你可以摸三张牌，然后变身为游戏外的一名蜀势力武将，若如此做，回合结束后，你死亡。",

	-- 贾诩
	["#jiawenhe"] = "明哲保身",
	["jiawenhe"] = "贾文和",
	["dongcha"] = "洞察",
	[":dongcha"] = "准备阶段开始时，你可以选择一名其他角色：其所有手牌于当前回合内对你可见。\
		<font color=\'purple\'>◆你以此法选择的角色的操作不公开，换言之，只有你知道洞察的目标。</font>",
	["dushi"] = "毒士",
	[":dushi"] = "<font color=\"blue\"><b>锁定技，</b></font>当你死亡时，杀死你的角色获得技能“崩坏”。",
	["@collapse"] = "崩坏",
	["@dongcha"] = "请选择“洞察”的目标",

	-- 古之恶来
	["#guzhielai"] = "不坠悍将",
	["guzhielai"] = "古之恶来",
	["sizhan"] = "死战",
	[":sizhan"] = "<font color=\"blue\"><b>锁定技，</b></font>每当你受到一次伤害时，你防止此伤害并获得等同于伤害点数的“死战”标记；结束阶段开始时，你失去等量于你拥有的“死战”标记数的体力并弃所有的“死战”标记。",
	["shenli"] = "神力",
	[":shenli"] = "<font color=\"blue\"><b>锁定技，</b></font>你于出牌阶段内<font color='red'>第一次</font>使用【杀】造成伤害时，此伤害+X（X为当前死战标记数且最大为3）。",
	["#SizhanPrevent"] = "%from 的锁定技【%arg2】被触发，防止了当前的 %arg 点伤害",
	["#SizhanLoseHP"] = "%from 的锁定技【%arg2】被触发，流失了 %arg 点体力",
	["#ShenliBuff"] = "%from 的锁定技【神力】被触发，【杀】的伤害增加了 %arg, 达到了 %arg2 点",
	["@struggle"] = "死战",

	-- 邓艾
	["#dengshizai"] = "破蜀首功",
	["dengshizai"] = "邓士载",
	["zhenggong"] = "争功",
	[":zhenggong"] = "其他角色的回合开始前，若你的武将牌正面朝上，你可以获得一个额外的回合，此回合结束后，你将武将牌翻面。",
	["toudu"] = "偷渡",
	[":toudu"] = "每当你受到一次伤害后，若你的武将牌背面朝上，你可以弃置一张手牌，将你的武将牌翻面，然后视为使用一张【杀】。",
	["@toudu"] = "你可以弃置一张牌发动技能“偷渡”",
	["~toudu"] = "选择一张牌→选择【杀】的目标→点击确定",

	-- 张鲁
	["#zhanggongqi"] = "五斗米道",
	["zhanggongqi"] = "张公祺",
	["ytyishe"] = "义舍",
	[":ytyishe"] = "出牌阶段，你可以将至少一张手牌置于你的武将牌上称为“米”（“米”不能多于五张）或获得至少一张“米”；其他角色的<font color=\"green\"><b>出牌阶段限两次，</b></font>其可选择一张“米”，你可以将之交给其。",
	["xiliang"] = "惜粮",
	[":xiliang"] = "每当其他角色于其弃牌阶段因弃置失去一张红色牌后，你可以选择一项：1.将之置于你的武将牌上，称为“米”；2.获得之。",
	["ytrice"] = "米",
	["xiliang:put"] = "收为“米”",
	["xiliang:obtain"] = "获得此牌",
	["ytyishe_ask"] = "义舍要牌",
	["ytyishe_ask:allow"] = "同意",
	["ytyishe_ask:disallow"] = "不同意",

	-- 倚天剑
	["#yitianjian"] = "跨海斩长鲸",
	["yitianjian"] = "倚天剑",
	["zhengfeng"] = "争锋",
	[":zhengfeng"] = "<font color=\"blue\"><b>锁定技，</b></font>若你的装备区没有武器牌，你的攻击范围为X（X为你的体力值）。",
	["ytzhenwei"] = "镇威",
	[":ytzhenwei"] = "每当你使用的【杀】被【闪】抵消时，你可以获得处理区里的此【闪】。",
	["yitian"] = "倚天",
	[":yitian"] = "<b>联动技</b>，每当你对曹操造成伤害时，你可以令该伤害-1。",
	["#YitianSolace"] = "%from 发动了技能“倚天”，对 %to 的 %arg 点伤害减至 %arg2 点",

	-- 庞德
	["#panglingming"] = "枱榇之悟",
	["panglingming"] = "庞令明",
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以失去1点体力或弃置一张武器牌，依次弃置你攻击范围内的一名角色区域内的两张牌。",

-- CV&Designer
	["designer:weiwudi"] = "官方内测第三版",
	["illustrator:weiwudi"] = "三国志大战",
	["cv:weiwudi"] = "",

	["designer:caochong"] = "太阳神上",
	["illustrator:caochong"] = "三国志大战",
	["cv:caochong"] = "",

	["designer:zhangjunyi"] = "孔孟老庄胡",
	["illustrator:zhangjunyi"] = "火凤燎原",
	["cv:zhangjunyi"] = "",

	["designer:lukang"] = "太阳神上",
	["illustrator:lukang"] = "火神原画",
	["cv:lukang"] = "",

	["designer:jinxuandi"] = "title2009,塞克洛",
	["illustrator:jinxuandi"] = "梦三国",
	["cv:jinxuandi"] = "宇文天启",
	["$wuling1"] = "长虹贯日，火舞旋风",
	["$wuling2"] = "追云逐电，雷动九天",
	["$wuling3"] = "云销雨霁，彩彻区明",
	["$wuling4"] = "举火燎天，星沉地动",
	["$wuling5"] = "大地光华，承天载物",
	["~jinxuandi"] = "千年恩怨，一笔勾销，历史轮回，转身忘掉",

	["designer:xiahoujuan"] = "宇文天启，艾艾艾",
	["illustrator:xiahoujuan"] = "三国志大战",
	["cv:xiahoujuan"] = "妙妙",
	["$lianli1"] = "连理并蒂，比翼不移",
	["$lianli2"] = "陟彼南山，言采其樵。未见君子，忧心惙惙",
	["$tongxin"] = "执子之手，与子偕老",
	["~xiahoujuan"] = "行与子逝兮，归于其室",

	["designer:caizhaoji"] = "冢冢的青藤",
	["illustrator:caizhaoji"] = "火星时代实训基地",
	["cv:caizhaoji"] = "妙妙",
	["$guihan"] = "雁南征兮欲寄边心，雁北归兮为得汉音",
	["$caizhaoji_hujia"] = "北风厉兮肃泠泠。胡笳动兮边马鸣",
	["~caizhaoji"] = "人生几何时，怀忧终年岁",

	["designer:luboyan"] = "太阳神上、冢冢的青藤",
	["illustrator:luboyan"] = "真三国无双5",
	["cv:luboyan"] = "水浒杀神火将魏定国",
	["designer:luboyanf"] = "太阳神上、冢冢的青藤",
	["illustrator:luboyanf"] = "阿摸",
	["cv:luboyanf"] = "",
	["$shaoying1"] = "烈焰升腾，万物尽毁！",
	["$shaoying2"] = "以火应敌，贼人何处逃窜？！",
	["$zonghuo"] = "（燃烧声）",
	["~luboyan"] = "玩火自焚啊！",

	["designer:zhongshiji"] = "Jr. Wakaran",
	["illustrator:zhongshiji"] = "战国无双3",
	["cv:zhongshiji"] = "",

	["designer:jiangboyue"] = "Jr. Wakaran, 太阳神上",
	["illustrator:jiangboyue"] = "不详",
	["cv:jiangboyue"] = "Jr. Wakaran",
	["$lexue1"] = "勤习出奇策,乐学生妙计",
	["$lexue2"] = "此乃五虎上将之勇",
	["$lexue3"] = "此乃诸葛武侯之智",
	["$xunzhi1"] = "丞相,计若不成,维亦无悔!",
	["$xunzhi2"] = "蜀汉英烈,忠魂佑我!",
	["$XunzhiAnimate"] = "image=image/animate/xunzhi.png",
	["~jiangboyue"] = "吾计不成,乃天命也!",

	["designer:jiawenhe"] = "氢弹",
	["illustrator:jiawenhe"] = "三国豪杰传",
	["cv:jiawenhe"] = "",

	["designer:guzhielai"] = "Jr. Wakaran, 太阳神上",
	["illustrator:guzhielai"] = "火凤燎原",
	["cv:guzhielai"] = "",

	["designer:dengshizai"] = "Bu懂",
	["illustrator:dengshizai"] = "三国豪杰传",
	["cv:dengshizai"] = "阿澈",
	["$zhenggong"] = "不肯屈人后,看某第一功",
	["$toudu"] = "攻其不意,掩其无备",
	["~dengshizai"] = "蹇利西南,不利东北；破蜀功高,难以北回……",

	["designer:zhanggongqi"] = "背碗卤粉",
	["illustrator:zhanggongqi"] = "真三国友盟",
	["cv:zhanggongqi"] = "",

	["designer:yitianjian"] = "太阳神上",
	["illustrator:yitianjian"] = "轩辕剑",
	["cv:yitianjian"] = "",

	["designer:panglingming"] = "太阳神上",
	["illustrator:panglingming"] = "三国志大战",
	["cv:panglingming"] = "乱天乱外",
	["$taichen"] = "良将不惧死以苟免，烈士不毁节以求生",
	["~panglingming"] = "吾宁死于刀下，岂降汝乎",
}

