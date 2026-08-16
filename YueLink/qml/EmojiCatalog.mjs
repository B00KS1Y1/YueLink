export const packageId = "iconfont-4116"
export const resourcePrefix = "qrc:/yuelink/assets/emoji/"

export const emojis = [
    { emojiId: "1840111", name: "中毒", file: "01-poisoned.svg" },
    { emojiId: "1840112", name: "天使", file: "02-angel.svg" },
    { emojiId: "1840113", name: "中毒-1", file: "03-poisoned-1.svg" },
    { emojiId: "1840114", name: "酷", file: "04-cool.svg" },
    { emojiId: "1840115", name: "迷茫", file: "05-confused.svg" },
    { emojiId: "1840116", name: "生气", file: "06-angry.svg" },
    { emojiId: "1840117", name: "酷-1", file: "07-cool-1.svg" },
    { emojiId: "1840118", name: "头晕", file: "08-dizzy.svg" },
    { emojiId: "1840119", name: "哭", file: "09-crying.svg" },
    { emojiId: "1840120", name: "哭-1", file: "10-crying-1.svg" },
    { emojiId: "1840121", name: "面无表情", file: "11-expressionless.svg" },
    { emojiId: "1840122", name: "魔鬼", file: "12-devil.svg" },
    { emojiId: "1840123", name: "懵B", file: "13-stunned.svg" },
    { emojiId: "1840124", name: "开心-1", file: "14-happy-1.svg" },
    { emojiId: "1840125", name: "开心-2", file: "15-happy-2.svg" },
    { emojiId: "1840126", name: "开心", file: "16-happy.svg" },
    { emojiId: "1840127", name: "受伤", file: "17-injured.svg" },
    { emojiId: "1840128", name: "笑哭", file: "18-laughing-crying.svg" },
    { emojiId: "1840129", name: "热恋", file: "19-in-love.svg" },
    { emojiId: "1840130", name: "亲吻", file: "20-kiss.svg" },
    { emojiId: "1840131", name: "亲吻-2", file: "21-kiss-2.svg" },
    { emojiId: "1840132", name: "亲吻-1", file: "22-kiss-1.svg" },
    { emojiId: "1840133", name: "口罩", file: "23-mask.svg" },
    { emojiId: "1840134", name: "面无表情-1", file: "24-expressionless-1.svg" },
    { emojiId: "1840135", name: "静音", file: "25-silent.svg" },
    { emojiId: "1840136", name: "难过", file: "26-sad.svg" },
    { emojiId: "1840137", name: "难过-1", file: "27-sad-1.svg" },
    { emojiId: "1840138", name: "害怕", file: "28-afraid.svg" },
    { emojiId: "1840139", name: "闭嘴", file: "29-shut-mouth.svg" },
    { emojiId: "1840140", name: "害怕-1", file: "30-afraid-1.svg" },
    { emojiId: "1840141", name: "震惊-1", file: "31-shocked-1.svg" },
    { emojiId: "1840142", name: "生病", file: "32-sick.svg" },
    { emojiId: "1840143", name: "笑", file: "33-laugh.svg" },
    { emojiId: "1840144", name: "笑-1", file: "34-laugh-1.svg" },
    { emojiId: "1840145", name: "睡觉", file: "35-sleeping.svg" },
    { emojiId: "1840146", name: "微笑-1", file: "36-smile-1.svg" },
    { emojiId: "1840147", name: "眼红", file: "37-jealous.svg" },
    { emojiId: "1840148", name: "流汗", file: "38-sweating.svg" },
    { emojiId: "1840149", name: "震惊", file: "39-shocked.svg" },
    { emojiId: "1840150", name: "奸笑", file: "40-smirk.svg" },
    { emojiId: "1840151", name: "思考", file: "41-thinking.svg" },
    { emojiId: "1840152", name: "疲惫", file: "42-tired.svg" },
    { emojiId: "1840153", name: "吐舌", file: "43-tongue-out.svg" },
    { emojiId: "1840154", name: "吐舌-1", file: "44-tongue-out-1.svg" },
    { emojiId: "1840155", name: "吐舌-2", file: "45-tongue-out-2.svg" },
    { emojiId: "1840156", name: "斜眼", file: "46-side-eye.svg" },
    { emojiId: "1840157", name: "眨眼", file: "47-wink.svg" },
    { emojiId: "1840158", name: "呕吐", file: "48-vomiting.svg" },
    { emojiId: "1840159", name: "僵尸", file: "49-zombie.svg" },
    { emojiId: "1840160", name: "呕吐-1", file: "50-vomiting-1.svg" }
]

export function sourceFor(requestedPackageId, emojiId) {
    if (requestedPackageId !== packageId)
        return ""

    for (let index = 0; index < emojis.length; ++index) {
        if (emojis[index].emojiId === emojiId)
            return resourcePrefix + emojis[index].file
    }
    return ""
}
