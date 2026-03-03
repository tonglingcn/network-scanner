#include "ipscannerthread.h"

#include <QElapsedTimer>
#include <QDebug>
#include <QProcess>
#include <QHostInfo>
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QNetworkInterface>
#include <QThread>
#include <QCryptographicHash>

// 简化的 OUI 数据库 (MAC 地址前缀 -> 厂商)
static const QMap<QString, QString> OUI_DATABASE = {
    // 常见厂商
    {"00:0C:29", "VMware, Inc."},
    {"00:50:56", "VMware, Inc."},
    {"00:1A:4A", "Dell Inc."},
    {"00:1B:21", "Dell Inc."},
    {"00:E0:4C", "Realtek Semiconductor"},
    {"00:1B:63", "Apple, Inc."},
    {"00:1F:F3", "Apple, Inc."},
    {"00:25:00", "Apple, Inc."},
    {"00:15:99", "Apple, Inc."},
    {"00:1A:11", "Apple, Inc."},
    {"00:03:93", "Apple, Inc."},
    {"00:0A:95", "Apple, Inc."},
    {"00:0D:93", "Apple, Inc."},
    {"BC:D1:D3", "Apple, Inc."},
    {"F8:FF:C2", "Apple, Inc."},
    {"A4:D1:D2", "Apple, Inc."},
    {"3C:15:C2", "Apple, Inc."},
    {"34:36:3B", "Apple, Inc."},
    {"AC:87:A3", "Apple, Inc."},
    {"AC:BC:32", "Apple, Inc."},
    {"E4:B3:18", "Apple, Inc."},
    {"00:0C:29", "VMware, Inc."},
    {"00:50:56", "VMware, Inc."},
    {"00:05:69", "VMware, Inc."},
    {"00:1C:14", "VMware, Inc."},
    {"00:16:3E", "XenSource, Inc."},
    {"52:54:00", "QEMU Virtual NIC"},
    {"08:00:27", "Cadmus Computer Systems"},
    {"00:15:5D", "Microsoft Corporation"},
    {"00:1D:D8", "Microsoft Corporation"},
    {"00:26:2D", "Intel Corporate"},
    {"00:1B:21", "Intel Corporate"},
    {"00:23:AE", "Intel Corporate"},
    {"00:25:B3", "Intel Corporate"},
    {"00:26:B9", "Intel Corporate"},
    {"00:1E:67", "Intel Corporate"},
    {"00:E0:4C", "Realtek Semiconductor"},
    {"B8:88:E3", "Realtek Semiconductor"},
    {"00:26:18", "Realtek Semiconductor"},
    {"A4:83:E7", "Realtek Semiconductor"},
    {"00:18:82", "Broadcom Corporation"},
    {"00:22:B0", "Broadcom Corporation"},
    {"00:1E:C9", "Broadcom Corporation"},
    {"00:1A:4B", "Broadcom Corporation"},
    {"00:17:C4", "Broadcom Corporation"},
    {"00:15:17", "Broadcom Corporation"},
    {"00:14:A5", "Broadcom Corporation"},
    {"00:13:D4", "Broadcom Corporation"},
    {"00:11:95", "Broadcom Corporation"},
    {"00:10:18", "Broadcom Corporation"},
    {"00:0F:B5", "Broadcom Corporation"},
    {"00:0E:7B", "Broadcom Corporation"},
    {"00:0D:B6", "Broadcom Corporation"},
    {"00:0C:CF", "Broadcom Corporation"},
    {"00:0B:DB", "Broadcom Corporation"},
    {"00:0A:CD", "Broadcom Corporation"},
    {"00:09:BE", "Broadcom Corporation"},
    {"00:08:B7", "Broadcom Corporation"},
    {"00:07:E9", "Broadcom Corporation"},
    {"00:06:26", "Broadcom Corporation"},
    {"00:05:BA", "Broadcom Corporation"},
    {"00:04:ED", "Broadcom Corporation"},
    {"00:03:FF", "Broadcom Corporation"},
    {"00:02:A5", "Broadcom Corporation"},
    {"00:01:CA", "Broadcom Corporation"},
    {"00:00:BD", "Broadcom Corporation"},
    {"00:90:4B", "Cisco Systems"},
    {"00:11:BC", "Cisco Systems"},
    {"00:12:43", "Cisco Systems"},
    {"00:13:80", "Cisco Systems"},
    {"00:15:63", "Cisco Systems"},
    {"00:16:47", "Cisco Systems"},
    {"00:17:0F", "Cisco Systems"},
    {"00:17:94", "Cisco Systems"},
    {"00:18:BA", "Cisco Systems"},
    {"00:19:07", "Cisco Systems"},
    {"00:19:D1", "Cisco Systems"},
    {"00:1A:30", "Cisco Systems"},
    {"00:1A:A2", "Cisco Systems"},
    {"00:1B:0D", "Cisco Systems"},
    {"00:1B:D6", "Cisco Systems"},
    {"00:1C:28", "Cisco Systems"},
    {"00:1C:B4", "Cisco Systems"},
    {"00:1D:45", "Cisco Systems"},
    {"00:1D:70", "Cisco Systems"},
    {"00:1E:14", "Cisco Systems"},
    {"00:1E:49", "Cisco Systems"},
    {"00:1E:F7", "Cisco Systems"},
    {"00:23:04", "Cisco Systems"},
    {"00:27:0D", "Cisco Systems"},
    {"00:30:94", "Cisco Systems"},
    {"F4:8E:38", "TP-Link Technologies"},
    {"00:25:86", "TP-Link Technologies"},
    {"E8:4E:06", "TP-Link Technologies"},
    {"84:A8:E4", "TP-Link Technologies"},
    {"88:25:93", "TP-Link Technologies"},
    {"DC:41:A9", "TP-Link Technologies"},
    {"80:89:17", "TP-Link Technologies"},
    {"50:C7:BF", "TP-Link Technologies"},
    {"68:FF:7B", "TP-Link Technologies"},
    {"A0:F3:C1", "TP-Link Technologies"},
    {"20:DC:E6", "TP-Link Technologies"},
    {"C8:3A:35", "TP-Link Technologies"},
    {"D4:6E:0E", "TP-Link Technologies"},
    {"BC:23:3F", "TP-Link Technologies"},
    {"AC:84:C6", "TP-Link Technologies"},
    {"B8:27:EB", "Raspberry Pi Foundation"},
    {"DC:A6:32", "Raspberry Pi Foundation"},
    {"E4:5F:01", "Raspberry Pi Foundation"},
    // 新增常见厂商
    {"34:8D:52", "Sichuan Tianyi Comheart Telecom"},
    {"AC:C5:1B", "Zhuhai Pantum Electronics"},
    {"A8:13:06", "vivo Mobile Communication"},
    {"62:CB:FA", "Unknown"},
    {"00:1B:63", "Apple, Inc."},
    {"00:1C:B3", "Apple, Inc."},
    {"00:23:6C", "Apple, Inc."},
    {"00:23:DF", "Apple, Inc."},
    {"00:25:4B", "Apple, Inc."},
    {"04:0C:CE", "Apple, Inc."},
    {"34:15:9D", "Apple, Inc."},
    {"3C:07:54", "Apple, Inc."},
    {"40:A6:D9", "Apple, Inc."},
    {"58:55:CA", "Apple, Inc."},
    {"78:7C:34", "Apple, Inc."},
    {"8C:85:90", "Apple, Inc."},
    {"A4:C3:F0", "Apple, Inc."},
    {"A8:66:7F", "Apple, Inc."},
    {"AC:87:A3", "Apple, Inc."},
    {"B4:2E:99", "Apple, Inc."},
    {"C4:B3:01", "Apple, Inc."},
    {"CC:3A:61", "Apple, Inc."},
    {"D0:A6:32", "Apple, Inc."},
    {"D4:61:9E", "Apple, Inc."},
    {"E8:50:8B", "Apple, Inc."},
    {"F0:18:98", "Apple, Inc."},
    {"F0:F8:F2", "Apple, Inc."},
    {"F4:5C:89", "Apple, Inc."},
    {"FC:E9:98", "Apple, Inc."},
    {"98:01:A7", "HUAWEI Technologies Co.,Ltd."},
    {"88:25:93", "HUAWEI Technologies Co.,Ltd."},
    {"4C:54:99", "HUAWEI Technologies Co.,Ltd."},
    {"00:E0:FC", "HUAWEI Technologies Co.,Ltd."},
    {"34:C0:B1", "Honor Device Co., Ltd."},
    {"AC:D1:08", "Honor Device Co., Ltd."},
    {"D8:3A:DD", "Honor Device Co., Ltd."},
    {"78:11:DC", "Xiaomi Communications Co., Ltd."},
    {"34:CE:00", "Xiaomi Communications Co., Ltd."},
    {"F8:A4:5F", "Xiaomi Communications Co., Ltd."},
    {"64:09:80", "Xiaomi Communications Co., Ltd."},
    {"A8:5E:45", "Xiaomi Communications Co., Ltd."},
    {"AC:23:3F", "Xiaomi Communications Co., Ltd."},
    {"34:17:E9", "Xiaomi Communications Co., Ltd."},
    {"C0:EE:FB", "Xiaomi Communications Co., Ltd."},
    {"D4:61:9D", "Xiaomi Communications Co., Ltd."},
    {"28:CF:E9", "OPPO Guangdong Mobile Communications"},
    {"E4:9F:73", "OPPO Guangdong Mobile Communications"},
    {"D0:1A:50", "OPPO Guangdong Mobile Communications"},
    {"2C:F3:F2", "OPPO Guangdong Mobile Communications"},
    {"AA:BB:CC", "OPPO Guangdong Mobile Communications"},
    {"3A:2E:41", "OnePlus Technology (Shenzhen) Co., Ltd"},
    {"E0:94:67", "OnePlus Technology (Shenzhen) Co., Ltd"},
    {"C8:C3:45", "OnePlus Technology (Shenzhen) Co., Ltd"},
    {"6C:92:BF", "OnePlus Technology (Shenzhen) Co., Ltd"},
    {"A0:7D:C5", "Vivo Mobile Communication Co., Ltd"},
    {"A8:13:06", "Vivo Mobile Communication Co., Ltd"},
    {"CC:07:E6", "Vivo Mobile Communication Co., Ltd"},
    {"D0:86:1F", "Vivo Mobile Communication Co., Ltd"},
    {"D4:5A:8E", "Vivo Mobile Communication Co., Ltd"},
    {"F8:94:C2", "Vivo Mobile Communication Co., Ltd"},
    {"6A:2B:73", "Vivo Mobile Communication Co., Ltd"},
    {"8C:34:FD", "Vivo Mobile Communication Co., Ltd"},
    {"00:1E:58", "Samsung Electronics Co.,Ltd"},
    {"00:15:99", "Samsung Electronics Co.,Ltd"},
    {"00:16:6F", "Samsung Electronics Co.,Ltd"},
    {"00:17:C4", "Samsung Electronics Co.,Ltd"},
    {"00:21:CC", "Samsung Electronics Co.,Ltd"},
    {"00:22:43", "Samsung Electronics Co.,Ltd"},
    {"00:23:AE", "Samsung Electronics Co.,Ltd"},
    {"00:24:54", "Samsung Electronics Co.,Ltd"},
    {"00:25:65", "Samsung Electronics Co.,Ltd"},
    {"00:26:99", "Samsung Electronics Co.,Ltd"},
    {"00:E0:6C", "Samsung Electronics Co.,Ltd"},
    {"08:21:EF", "Samsung Electronics Co.,Ltd"},
    {"3C:D9:2B", "Samsung Electronics Co.,Ltd"},
    {"40:D2:8C", "Samsung Electronics Co.,Ltd"},
    {"4C:32:75", "Samsung Electronics Co.,Ltd"},
    {"5C:51:88", "Samsung Electronics Co.,Ltd"},
    {"6C:34:54", "Samsung Electronics Co.,Ltd"},
    {"8C:AE:4C", "Samsung Electronics Co.,Ltd"},
    {"A0:88:B4", "Samsung Electronics Co.,Ltd"},
    {"AC:FD:93", "Samsung Electronics Co.,Ltd"},
    {"B0:72:BF", "Samsung Electronics Co.,Ltd"},
    {"BC:D1:D3", "Samsung Electronics Co.,Ltd"},
    {"00:E0:4C", "Realtek Semiconductor"},
    {"00:E0:4D", "Realtek Semiconductor"},
    {"00:E0:4E", "Realtek Semiconductor"},
    {"00:1B:11", "Realtek Semiconductor"},
    {"00:1B:38", "Realtek Semiconductor"},
    {"00:1F:ED", "Realtek Semiconductor"},
    {"00:24:A8", "Realtek Semiconductor"},
    {"00:25:11", "Realtek Semiconductor"},
    {"00:26:5A", "Realtek Semiconductor"},
    {"00:30:18", "Realtek Semiconductor"},
    {"00:60:6E", "Realtek Semiconductor"},
    {"00:90:CC", "Realtek Semiconductor"},
    {"00:E0:4F", "Realtek Semiconductor"},
    {"02:CF:5C", "Realtek Semiconductor"},
    {"04:7D:7B", "Realtek Semiconductor"},
    {"0C:82:68", "Realtek Semiconductor"},
    {"10:BF:48", "Realtek Semiconductor"},
    {"14:CC:20", "Realtek Semiconductor"},
    {"1C:AF:F7", "Realtek Semiconductor"},
    {"20:89:84", "Realtek Semiconductor"},
    {"24:05:0F", "Realtek Semiconductor"},
    {"24:B6:FD", "Realtek Semiconductor"},
    {"2C:26:5F", "Realtek Semiconductor"},
    {"30:FC:68", "Realtek Semiconductor"},
    {"38:60:77", "Realtek Semiconductor"},
    {"3C:18:A0", "Realtek Semiconductor"},
    {"3C:97:0E", "Realtek Semiconductor"},
    {"40:8D:5C", "Realtek Semiconductor"},
    {"48:5B:39", "Realtek Semiconductor"},
    {"50:46:5D", "Realtek Semiconductor"},
    {"54:04:A6", "Realtek Semiconductor"},
    {"54:E1:AD", "Realtek Semiconductor"},
    {"58:8D:09", "Realtek Semiconductor"},
    {"60:45:CB", "Realtek Semiconductor"},
    {"60:6C:66", "Realtek Semiconductor"},
    {"60:E3:27", "Realtek Semiconductor"},
    {"64:70:02", "Realtek Semiconductor"},
    {"68:1D:EF", "Realtek Semiconductor"},
    {"70:5A:0F", "Realtek Semiconductor"},
    {"70:B3:D5", "Realtek Semiconductor"},
    {"70:BB:E9", "Realtek Semiconductor"},
    {"70:F9:6D", "Realtek Semiconductor"},
    {"74:27:EA", "Realtek Semiconductor"},
    {"78:24:AF", "Realtek Semiconductor"},
    {"7C:4C:A5", "Realtek Semiconductor"},
    {"80:86:3A", "Realtek Semiconductor"},
    {"84:A9:3E", "Realtek Semiconductor"},
    {"84:EF:18", "Realtek Semiconductor"},
    {"88:53:2E", "Realtek Semiconductor"},
    {"88:53:95", "Realtek Semiconductor"},
    {"88:AE:1D", "Realtek Semiconductor"},
    {"8C:16:FC", "Realtek Semiconductor"},
    {"8C:89:A5", "Realtek Semiconductor"},
    {"90:27:E4", "Realtek Semiconductor"},
    {"90:2B:34", "Realtek Semiconductor"},
    {"90:61:AE", "Realtek Semiconductor"},
    {"90:A4:DE", "Realtek Semiconductor"},
    {"90:F1:AA", "Realtek Semiconductor"},
    {"90:FB:A6", "Realtek Semiconductor"},
    {"94:DE:80", "Realtek Semiconductor"},
    {"98:3B:8F", "Realtek Semiconductor"},
    {"98:48:27", "Realtek Semiconductor"},
    {"98:90:96", "Realtek Semiconductor"},
    {"98:FA:9B", "Realtek Semiconductor"},
    {"9C:5C:8E", "Realtek Semiconductor"},
    {"9C:B6:54", "Realtek Semiconductor"},
    {"A0:1B:29", "Realtek Semiconductor"},
    {"A0:2C:F0", "Realtek Semiconductor"},
    {"A0:36:BC", "Realtek Semiconductor"},
    {"A0:40:A0", "Realtek Semiconductor"},
    {"A0:54:12", "Realtek Semiconductor"},
    {"A0:88:B4", "Realtek Semiconductor"},
    {"A0:B3:CC", "Realtek Semiconductor"},
    {"A0:C5:89", "Realtek Semiconductor"},
    {"A0:D3:C1", "Realtek Semiconductor"},
    {"A0:DE:61", "Realtek Semiconductor"},
    {"A4:83:E7", "Realtek Semiconductor"},
    {"A8:5E:45", "Realtek Semiconductor"},
    {"AC:1D:1C", "Realtek Semiconductor"},
    {"AC:22:0B", "Realtek Semiconductor"},
    {"AC:87:A3", "Realtek Semiconductor"},
    {"B0:25:AA", "Realtek Semiconductor"},
    {"B0:4E:26", "Realtek Semiconductor"},
    {"B4:0B:44", "Realtek Semiconductor"},
    {"B4:2E:99", "Realtek Semiconductor"},
    {"B8:27:EB", "Realtek Semiconductor"},
    {"B8:70:F4", "Realtek Semiconductor"},
    {"B8:88:E3", "Realtek Semiconductor"},
    {"BC:5F:F4", "Realtek Semiconductor"},
    {"BC:D1:D3", "Realtek Semiconductor"},
    {"C0:61:B8", "Realtek Semiconductor"},
    {"C0:C4:7C", "Realtek Semiconductor"},
    {"C4:8D:79", "Realtek Semiconductor"},
    {"C8:0A:A9", "Realtek Semiconductor"},
    {"C8:3A:35", "Realtek Semiconductor"},
    {"C8:60:00", "Realtek Semiconductor"},
    {"CC:32:E5", "Realtek Semiconductor"},
    {"CC:E1:A5", "Realtek Semiconductor"},
    {"D0:50:99", "Realtek Semiconductor"},
    {"D0:76:E7", "Realtek Semiconductor"},
    {"D0:88:A7", "Realtek Semiconductor"},
    {"D0:97:E3", "Realtek Semiconductor"},
    {"D4:81:D7", "Realtek Semiconductor"},
    {"D4:BE:D9", "Realtek Semiconductor"},
    {"D4:CA:6D", "Realtek Semiconductor"},
    {"D8:50:E6", "Realtek Semiconductor"},
    {"D8:5D:4C", "Realtek Semiconductor"},
    {"D8:61:23", "Realtek Semiconductor"},
    {"D8:67:C9", "Realtek Semiconductor"},
    {"DC:0E:A1", "Realtek Semiconductor"},
    {"DC:28:D8", "Realtek Semiconductor"},
    {"DC:53:60", "Realtek Semiconductor"},
    {"DC:A6:32", "Realtek Semiconductor"},
    {"DC:FB:4E", "Realtek Semiconductor"},
    {"E0:CB:4E", "Realtek Semiconductor"},
    {"E0:D5:5E", "Realtek Semiconductor"},
    {"E4:5F:01", "Realtek Semiconductor"},
    {"E8:4E:06", "Realtek Semiconductor"},
    {"F0:4D:A2", "Realtek Semiconductor"},
    {"F0:79:59", "Realtek Semiconductor"},
    {"F4:6D:04", "Realtek Semiconductor"},
    {"F4:B5:47", "Realtek Semiconductor"},
    {"F4:8E:38", "Realtek Semiconductor"},
    {"F4:EC:38", "Realtek Semiconductor"},
    {"F4:EC:EA", "Realtek Semiconductor"},
    {"F4:F1:C8", "Realtek Semiconductor"},
    {"F4:FF:57", "Realtek Semiconductor"},
    {"F8:FF:C2", "Realtek Semiconductor"},
    {"F4:8E:38", "TP-Link Technologies"},
    {"00:25:86", "TP-Link Technologies"},
    {"E8:4E:06", "TP-Link Technologies"},
    {"84:A8:E4", "TP-Link Technologies"},
    {"88:25:93", "TP-Link Technologies"},
    {"DC:41:A9", "TP-Link Technologies"},
    {"80:89:17", "TP-Link Technologies"},
    {"50:C7:BF", "TP-Link Technologies"},
    {"68:FF:7B", "TP-Link Technologies"},
    {"A0:F3:C1", "TP-Link Technologies"},
    {"20:DC:E6", "TP-Link Technologies"},
    {"C8:3A:35", "TP-Link Technologies"},
    {"D4:6E:0E", "TP-Link Technologies"},
    {"BC:23:3F", "TP-Link Technologies"},
    {"AC:84:C6", "TP-Link Technologies"},
    {"B8:27:EB", "Raspberry Pi Foundation"},
    {"DC:A6:32", "Raspberry Pi Foundation"},
    {"E4:5F:01", "Raspberry Pi Foundation"},
    {"34:8D:52", "Sichuan Tianyi Comheart Telecom"},
    {"AC:C5:1B", "Zhuhai Pantum Electronics"},
    {"A8:13:06", "vivo Mobile Communication"},
    {"62:CB:FA", "Unknown"},
    // 技嘉科技
    {"1C:1B:0D", "GIGA-BYTE TECHNOLOGY CO.,LTD."},
    {"00:1E:67", "GIGA-BYTE TECHNOLOGY CO.,LTD."},
    {"00:25:90", "GIGA-BYTE TECHNOLOGY CO.,LTD."},
    {"00:30:67", "GIGA-BYTE TECHNOLOGY CO.,LTD."},
    {"54:83:3A", "GIGA-BYTE TECHNOLOGY CO.,LTD."},
    // 微星
    {"00:15:60", "Micro-Star International Co., Ltd."},
    {"00:1B:21", "Micro-Star International Co., Ltd."},
    {"00:26:18", "Micro-Star International Co., Ltd."},
    // 华硕
    {"00:19:1B", "ASUSTek Computer Inc."},
    {"00:1B:FC", "ASUSTek Computer Inc."},
    {"00:21:CC", "ASUSTek Computer Inc."},
    {"00:24:8C", "ASUSTek Computer Inc."},
    {"00:26:18", "ASUSTek Computer Inc."},
    // 微软
    {"00:15:5D", "Microsoft Corporation"},
    {"00:1D:D8", "Microsoft Corporation"},
    {"E0:CA:94", "Microsoft Corporation"},
    // 戴尔
    {"00:1A:4A", "Dell Inc."},
    {"00:1B:21", "Dell Inc."},
    {"00:1E:C9", "Dell Inc."},
    {"00:21:70", "Dell Inc."},
    {"00:22:19", "Dell Inc."},
    {"00:23:AE", "Dell Inc."},
    {"00:24:B8", "Dell Inc."},
    {"00:25:64", "Dell Inc."},
    {"00:26:B9", "Dell Inc."},
    {"00:E0:4C", "Dell Inc."},
    {"14:FE:B5", "Dell Inc."},
    {"18:B7:09", "Dell Inc."},
    {"2C:76:8A", "Dell Inc."},
    {"3C:1C:68", "Dell Inc."},
    {"40:6C:8F", "Dell Inc."},
    {"6C:4B:90", "Dell Inc."},
    {"80:5E:C0", "Dell Inc."},
    {"84:2B:2B", "Dell Inc."},
    {"D0:67:E5", "Dell Inc."},
    {"E0:94:67", "Dell Inc."},
    {"F0:1F:AF", "Dell Inc."},
    // 英特尔
    {"00:00:C9", "Intel Corporate"},
    {"00:01:B7", "Intel Corporate"},
    {"00:02:B3", "Intel Corporate"},
    {"00:04:23", "Intel Corporate"},
    {"00:0E:0C", "Intel Corporate"},
    {"00:0E:A6", "Intel Corporate"},
    {"00:11:25", "Intel Corporate"},
    {"00:12:3F", "Intel Corporate"},
    {"00:13:02", "Intel Corporate"},
    {"00:14:22", "Intel Corporate"},
    {"00:15:17", "Intel Corporate"},
    {"00:15:58", "Intel Corporate"},
    {"00:15:00", "Intel Corporate"},
    {"00:16:76", "Intel Corporate"},
    {"00:16:6E", "Intel Corporate"},
    {"00:16:EA", "Intel Corporate"},
    {"00:17:08", "Intel Corporate"},
    {"00:18:6D", "Intel Corporate"},
    {"00:19:D1", "Intel Corporate"},
    {"00:1A:11", "Intel Corporate"},
    {"00:1B:21", "Intel Corporate"},
    {"00:1B:38", "Intel Corporate"},
    {"00:1C:25", "Intel Corporate"},
    {"00:1C:BF", "Intel Corporate"},
    {"00:1D:09", "Intel Corporate"},
    {"00:1E:67", "Intel Corporate"},
    {"00:1F:16", "Intel Corporate"},
    {"00:21:5E", "Intel Corporate"},
    {"00:21:70", "Intel Corporate"},
    {"00:21:85", "Intel Corporate"},
    {"00:21:86", "Intel Corporate"},
    {"00:21:CC", "Intel Corporate"},
    {"00:22:19", "Intel Corporate"},
    {"00:23:AE", "Intel Corporate"},
    {"00:25:B3", "Intel Corporate"},
    {"00:25:B5", "Intel Corporate"},
    {"00:25:90", "Intel Corporate"},
    {"00:26:B9", "Intel Corporate"},
    {"00:26:C6", "Intel Corporate"},
    {"00:26:C7", "Intel Corporate"},
    {"00:30:C1", "Intel Corporate"},
    {"00:30:48", "Intel Corporate"},
    {"00:50:8D", "Intel Corporate"},
    {"00:A0:C9", "Intel Corporate"},
    {"00:E0:4C", "Intel Corporate"},
    {"00:E0:4D", "Intel Corporate"},
    {"00:E0:ED", "Intel Corporate"},
    {"02:00:4C", "Intel Corporate"},
    {"02:15:17", "Intel Corporate"},
    {"02:21:CC", "Intel Corporate"},
    {"02:23:AE", "Intel Corporate"},
    {"02:E0:4C", "Intel Corporate"},
    {"02:E0:4D", "Intel Corporate"},
    {"02:E0:ED", "Intel Corporate"},
    {"02:FD:43", "Intel Corporate"},
    {"06:00:6C", "Intel Corporate"},
    {"06:FD:43", "Intel Corporate"},
    {"12:CD:02", "Intel Corporate"},
    {"14:DA:E9", "Intel Corporate"},
    {"1C:6F:65", "Intel Corporate"},
    {"1C:C1:DE", "Intel Corporate"},
    {"20:0E:C6", "Intel Corporate"},
    {"20:89:84", "Intel Corporate"},
    {"24:FD:52", "Intel Corporate"},
    {"26:A4:3C", "Intel Corporate"},
    {"2C:76:8A", "Intel Corporate"},
    {"30:E4:DB", "Intel Corporate"},
    {"30:F7:0D", "Intel Corporate"},
    {"3C:05:5C", "Intel Corporate"},
    {"3C:97:0E", "Intel Corporate"},
    {"3C:D9:2B", "Intel Corporate"},
    {"3C:E1:A1", "Intel Corporate"},
    {"40:D2:8C", "Intel Corporate"},
    {"4C:72:B9", "Intel Corporate"},
    {"4E:72:B9", "Intel Corporate"},
    {"50:3E:AA", "Intel Corporate"},
    {"50:6B:4B", "Intel Corporate"},
    {"50:7B:9D", "Intel Corporate"},
    {"50:EB:F6", "Intel Corporate"},
    {"52:54:00", "Intel Corporate"},
    {"54:12:4E", "Intel Corporate"},
    {"54:EF:44", "Intel Corporate"},
    {"56:84:EC", "Intel Corporate"},
    {"58:8D:09", "Intel Corporate"},
    {"58:B0:35", "Intel Corporate"},
    {"58:EF:20", "Intel Corporate"},
    {"60:03:08", "Intel Corporate"},
    {"60:45:CB", "Intel Corporate"},
    {"60:A4:4C", "Intel Corporate"},
    {"60:D8:19", "Intel Corporate"},
    {"64:D1:54", "Intel Corporate"},
    {"66:04:9A", "Intel Corporate"},
    {"66:F0:9D", "Intel Corporate"},
    {"68:05:CA", "Intel Corporate"},
    {"68:05:CB", "Intel Corporate"},
    {"6C:4B:90", "Intel Corporate"},
    {"6C:62:6D", "Intel Corporate"},
    {"6C:C9:84", "Intel Corporate"},
    {"6C:D1:F5", "Intel Corporate"},
    {"70:71:BC", "Intel Corporate"},
    {"70:85:C2", "Intel Corporate"},
    {"70:E6:8B", "Intel Corporate"},
    {"74:86:7A", "Intel Corporate"},
    {"78:A3:E4", "Intel Corporate"},
    {"78:A3:E5", "Intel Corporate"},
    {"78:B2:FD", "Intel Corporate"},
    {"7C:05:07", "Intel Corporate"},
    {"7C:D1:C3", "Intel Corporate"},
    {"80:A2:68", "Intel Corporate"},
    {"80:C6:F0", "Intel Corporate"},
    {"84:2B:2B", "Intel Corporate"},
    {"84:A8:E4", "Intel Corporate"},
    {"84:E0:42", "Intel Corporate"},
    {"88:B9:9D", "Intel Corporate"},
    {"88:C9:6C", "Intel Corporate"},
    {"8C:70:5A", "Intel Corporate"},
    {"90:E2:BA", "Intel Corporate"},
    {"90:F1:AA", "Intel Corporate"},
    {"90:FB:A6", "Intel Corporate"},
    {"94:C6:91", "Intel Corporate"},
    {"96:57:85", "Intel Corporate"},
    {"98:90:96", "Intel Corporate"},
    {"98:FA:9B", "Intel Corporate"},
    {"9C:93:4E", "Intel Corporate"},
    {"9C:D6:43", "Intel Corporate"},
    {"A0:36:9F", "Intel Corporate"},
    {"A0:C9:A0", "Intel Corporate"},
    {"A0:D3:C1", "Intel Corporate"},
    {"A0:F8:49", "Intel Corporate"},
    {"A0:FB:60", "Intel Corporate"},
    {"A4:C3:F0", "Intel Corporate"},
    {"AC:7B:A1", "Intel Corporate"},
    {"AC:81:3A", "Intel Corporate"},
    {"AC:87:A3", "Intel Corporate"},
    {"B8:88:E3", "Intel Corporate"},
    {"B8:CA:3A", "Intel Corporate"},
    {"BC:5F:F4", "Intel Corporate"},
    {"BC:D1:D3", "Intel Corporate"},
    {"C0:62:6B", "Intel Corporate"},
    {"C0:C4:7C", "Intel Corporate"},
    {"C0:EE:FB", "Intel Corporate"},
    {"C4:8D:79", "Intel Corporate"},
    {"CC:16:7E", "Intel Corporate"},
    {"CC:32:E5", "Intel Corporate"},
    {"CC:3A:61", "Intel Corporate"},
    {"CC:40:D0", "Intel Corporate"},
    {"D0:50:99", "Intel Corporate"},
    {"D0:67:E5", "Intel Corporate"},
    {"D0:B7:C1", "Intel Corporate"},
    {"D0:C9:82", "Intel Corporate"},
    {"D4:81:D7", "Intel Corporate"},
    {"D4:AE:52", "Intel Corporate"},
    {"D4:D7:47", "Intel Corporate"},
    {"D8:9E:F3", "Intel Corporate"},
    {"DA:1A:19", "Intel Corporate"},
    {"DC:3A:5E", "Intel Corporate"},
    {"DC:4A:3E", "Intel Corporate"},
    {"DC:7B:94", "Intel Corporate"},
    {"DE:AD:BE", "Intel Corporate"},
    {"E0:43:DB", "Intel Corporate"},
    {"E0:CA:94", "Intel Corporate"},
    {"E0:D5:5E", "Intel Corporate"},
    {"E4:D5:3D", "Intel Corporate"},
    {"E8:04:62", "Intel Corporate"},
    {"E8:39:35", "Intel Corporate"},
    {"E8:39:DF", "Intel Corporate"},
    {"E8:50:8B", "Intel Corporate"},
    {"E8:94:F6", "Intel Corporate"},
    {"EC:0D:9A", "Intel Corporate"},
    {"EC:F4:BB", "Intel Corporate"},
    {"F0:4D:A2", "Intel Corporate"},
    {"F0:79:59", "Intel Corporate"},
    {"F0:D4:BF", "Intel Corporate"},
    {"F4:6D:04", "Intel Corporate"},
    {"F4:8E:38", "Intel Corporate"},
    {"F8:FF:C2", "Intel Corporate"},
    {"FC:AA:14", "Intel Corporate"},
    {"FC:C2:3D", "Intel Corporate"},
    {"FE:C2:3D", "Intel Corporate"},
};

IPScannerThread::IPScannerThread(const QString &ip, QObject *parent)
    : QObject(parent)
    , QRunnable()
    , m_ip(ip)
{
    setAutoDelete(false);
}

IPScannerThread::~IPScannerThread()
{
}

void IPScannerThread::run()
{
    DeviceInfo info;
    info.ip = m_ip;

    qint64 responseTime = -1;
    info.isAvailable = pingIp(responseTime);
    info.responseTime = responseTime;

    if (info.isAvailable) {
        // 获取 MAC 地址
        info.macAddress = getMacAddress(m_ip);

        // 解析主机名
        info.hostname = resolveHostname(m_ip);

        // 获取厂商信息（基于 MAC 地址前缀）
        info.vendor = getVendorFromMac(info.macAddress);

        // 获取网卡厂商（基于 MAC 地址前缀，与 vendor 相同）
        info.nicVendor = getNicVendor(info.macAddress);
    }

    emit scanComplete(info);
}

bool IPScannerThread::pingIp(qint64 &responseTime)
{
    QElapsedTimer timer;
    timer.start();

#ifdef Q_OS_LINUX
    // 使用 ping 命令
    QProcess pingProcess;
    QString command = "ping";
    QStringList arguments;

    arguments << "-c" << "1"
             << "-W" << "1"
             << "-w" << "1"
             << "-n"
             << m_ip;

    pingProcess.start(command, arguments);
    bool finished = pingProcess.waitForFinished(1500);

    if (finished) {
        int exitCode = pingProcess.exitCode();
        responseTime = timer.elapsed();

        if (exitCode == 0) {
            return true;
        }
    }
#else
    // 回退方法
    QTcpSocket socket;
    socket.connectToHost(m_ip, 80);
    if (socket.waitForConnected(1000)) {
        responseTime = timer.elapsed();
        socket.disconnectFromHost();
        return true;
    }
#endif

    responseTime = -1;
    return false;
}

QString IPScannerThread::getMacAddress(const QString &ip)
{
    // 1. 检查是否是本机IP，如果是则从网络接口获取
    QString localMac = getLocalMacForIp(ip);
    if (!localMac.isEmpty()) {
        return localMac;
    }

    // 2. 首先尝试从 ARP 表获取
    QString mac = parseMacFromArp(ip);
    if (!mac.isEmpty() && mac != "未知") {
        return mac;
    }

    // 3. 先 ping 主机确保 ARP 缓存中存在该条目
    sendArpRequest(ip);

    // 4. 等待 ARP 表更新
    QThread::msleep(200);

    // 5. 再次尝试从 ARP 表获取
    mac = parseMacFromArp(ip);
    if (!mac.isEmpty() && mac != "未知") {
        return mac;
    }

    // 6. 发送额外的 ping 请求（最多 2 次）
    for (int i = 0; i < 2; i++) {
        sendArpRequest(ip);
        QThread::msleep(150);
        mac = parseMacFromArp(ip);
        if (!mac.isEmpty() && mac != "未知") {
            return mac;
        }
    }

    // 7. 尝试使用 arping 命令（如果可用）
    mac = getMacViaArping(ip);
    if (!mac.isEmpty() && mac != "未知") {
        return mac;
    }

    return "未知";
}

QString IPScannerThread::parseMacFromArp(const QString &ip)
{
    QProcess arpProcess;
    arpProcess.start("arp", QStringList() << "-n");
    if (!arpProcess.waitForFinished(2000)) {
        return "";
    }

    QString output = arpProcess.readAllStandardOutput();
    QStringList lines = output.split('\n');

    // 解析 ARP 输出
    // Linux 格式: IP address       HWtype     HWaddress           Flags Mask            Iface
    // 示例: 192.168.1.1    ether      34:8d:52:a3:55:be   C                     eth0
    // 示例: 192.168.1.201 (incomplete)                     enp2s0
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("Address")) {
            continue;
        }

        QStringList parts = trimmedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        // 检查是否匹配目标 IP
        if (parts.isEmpty() || parts[0] != ip) {
            continue;
        }

        // 检查是否有 "incomplete" 标记
        if (parts.contains("(incomplete)")) {
            continue; // 跳过不完整的 ARP 条目
        }

        // 查找 MAC 地址字段（通常在第 3 或第 4 个位置）
        for (const QString &part : parts) {
            if (part.contains(':') && part.count(':') == 5) {
                // 验证 MAC 地址格式
                QRegularExpression macRegex("^[0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5}$");
                if (macRegex.match(part).hasMatch()) {
                    return part.toUpper();
                }
            }
        }
    }

    return "";
}

QString IPScannerThread::getMacViaArping(const QString &ip)
{
    QProcess arpingProcess;
    arpingProcess.start("arping", QStringList() << "-c" << "1" << "-w" << "1" << ip);
    if (!arpingProcess.waitForFinished(1500)) {
        return "";
    }

    QString output = arpingProcess.readAllStandardOutput() + "\n" + arpingProcess.readAllStandardError();

    // 解析 arping 输出中的 MAC 地址
    // 格式: Unicast reply from 192.168.1.1 [34:8D:52:A3:55:BE]  0.000ms
    QRegularExpression macRegex("\\[([0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2})\\]");
    QRegularExpressionMatch match = macRegex.match(output);

    if (match.hasMatch()) {
        return match.captured(1).toUpper();
    }

    return "";
}

void IPScannerThread::sendArpRequest(const QString &ip)
{
    // 发送 ping 来触发 ARP 请求
    QProcess pingProcess;
    pingProcess.start("ping", QStringList() << "-c" << "1" << "-W" << "1" << ip);
    pingProcess.waitForFinished(1000);
}

QString IPScannerThread::resolveHostname(const QString &ip)
{
    // 1. 首先检查是否是本机IP
    QString localHostname = getLocalHostnameForIp(ip);
    if (!localHostname.isEmpty() && localHostname != "未知") {
        return localHostname;
    }

    // 2. 使用 QHostInfo 进行反向 DNS 查询
    QHostInfo hostInfo = QHostInfo::fromName(ip);

    if (hostInfo.error() == QHostInfo::NoError && !hostInfo.hostName().isEmpty()) {
        QString hostname = hostInfo.hostName();

        // 检查是否只是IP地址（说明DNS查询失败）
        QHostAddress testAddr(hostname);
        if (!testAddr.isNull()) {
            // 如果返回的是IP地址，尝试使用 nslookup
            return resolveHostnameViaNslookup(ip);
        }

        // 移除域名后缀，只保留主机名
        int dotPos = hostname.indexOf('.');
        if (dotPos > 0) {
            hostname = hostname.left(dotPos);
        }

        // 简单验证主机名
        if (hostname.length() > 0 && hostname.length() < 64) {
            return hostname;
        }
    }

    // 3. 尝试使用 nslookup 命令
    QString nslookupResult = resolveHostnameViaNslookup(ip);
    if (!nslookupResult.isEmpty() && nslookupResult != "未知") {
        return nslookupResult;
    }

    return "未知";
}

QString IPScannerThread::getLocalMacForIp(const QString &ip)
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp &&
            interface.flags() & QNetworkInterface::IsRunning &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {

            QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : addressEntries) {
                if (entry.ip().toString() == ip) {
                    return interface.hardwareAddress().toUpper();
                }
            }
        }
    }
    return "";
}

QString IPScannerThread::getLocalHostnameForIp(const QString &ip)
{
    Q_UNUSED(ip);
    // 获取本机主机名
    QProcess process;
    process.start("hostname", QStringList() << "-f");
    if (process.waitForFinished(1000)) {
        QString hostname = process.readAllStandardOutput().trimmed();

        // 移除域名后缀，只保留主机名
        int dotPos = hostname.indexOf('.');
        if (dotPos > 0) {
            hostname = hostname.left(dotPos);
        }

        if (!hostname.isEmpty() && hostname != "localhost") {
            return hostname;
        }
    }
    return "";
}

QString IPScannerThread::resolveHostnameViaNslookup(const QString &ip)
{
    QProcess nslookupProcess;
    nslookupProcess.start("nslookup", QStringList() << ip);
    if (!nslookupProcess.waitForFinished(2000)) {
        return "";
    }

    QString output = nslookupProcess.readAllStandardOutput();

    // 解析 nslookup 输出
    // 格式: server = 127.0.0.53
    //        Address: 127.0.0.53#53
    //
    //        2.168.1.7.in-addr.arpa    name = deepin-PC.
    QRegularExpression nameRegex("name\\s*=\\s*([^\\.\\s]+)");
    QRegularExpressionMatch match = nameRegex.match(output);

    if (match.hasMatch()) {
        QString hostname = match.captured(1);
        // 移除末尾的点
        hostname = hostname.remove('.');
        if (!hostname.isEmpty()) {
            return hostname;
        }
    }

    // 尝试另一种格式: hostname = xxx.xxx
    nameRegex.setPattern("=\\s*([a-zA-Z0-9-]+)\\.");
    match = nameRegex.match(output);

    if (match.hasMatch()) {
        return match.captured(1);
    }

    return "";
}

QString IPScannerThread::getVendorFromMac(const QString &mac)
{
    if (mac.isEmpty() || mac == "未知") {
        return "未知";
    }

    // 提取 MAC 地址前缀（前 3 组，即前 6 位）
    QString prefix;
    QStringList parts = mac.split(':');

    if (parts.size() >= 3) {
        prefix = QString("%1:%2:%3")
            .arg(parts[0])
            .arg(parts[1])
            .arg(parts[2]);
        prefix = prefix.toUpper();

        qDebug() << "Looking up MAC prefix:" << prefix;

        // 查找 OUI 数据库
        if (OUI_DATABASE.contains(prefix)) {
            QString vendor = OUI_DATABASE[prefix];
            qDebug() << "Found vendor for" << prefix << ":" << vendor;
            return vendor;
        }
    }

    qDebug() << "Vendor not found for MAC:" << mac;
    return "未知厂商";
}

QString IPScannerThread::getNicVendor(const QString &mac)
{
    // 网卡厂商通常与 MAC 地址厂商相同
    return getVendorFromMac(mac);
}

QString IPScannerThread::loadOuiDatabase()
{
    // 这里可以添加从文件加载 OUI 数据库的逻辑
    // 目前使用内置的数据库
    return "";
}

QString IPScannerThread::lookupVendor(const QString &macPrefix)
{
    return OUI_DATABASE.value(macPrefix, "未知厂商");
}
