# 活动管理子系统设计（成员 C）

## 1. 类图

```mermaid
classDiagram
    class Activity {
        <<abstract>>
        -string activityId
        -string clubId
        -string activityName
        -vector~string~ tags
        -string startTime
        -string endTime
        -string location
        -int maxPeople
        -string status
        -vector~string~ participants
        -vector~string~ waitList
        +addParticipant(uid) bool
        +removeParticipant(uid) bool
        +addWait(uid) bool
        +popWait() string
        +overlaps(other) bool
        +isValidTime(t)$ bool
        +getType()* string
        +getExtra()* string
        +calcPoints()* int
        +showInfo()* void
        +toFileString() string
    }
    class Lecture {
        -string speaker
        +calcPoints() int  "5"
    }
    class Competition {
        -string prize
        +calcPoints() int  "10"
    }
    class Performance {
        -string programType
        +calcPoints() int  "8"
    }
    class ActivityManager {
        -vector~unique_ptr~Activity~~ activities
        +load() bool
        +save() bool
        +publishActivity(clubId) bool
        +modifyActivity(activityId) bool
        +deleteActivity(activityId) bool
        +removeActivity(activityId) bool
        +auditActivity(activityId) bool
        +joinActivity(userId, activityId) bool
        +cancelActivity(userId, activityId) bool
        +showMyActivities(userId) void
        +showRegistration(activityId) void
        +manageRegistration(activityId) void
        +recommendActivity(interests) void
        +findConflict(userId, target) Activity*
        +getUserPoints(userId) int
    }
    Activity <|-- Lecture
    Activity <|-- Competition
    Activity <|-- Performance
    ActivityManager o-- Activity : 管理多个
```

## 2. 报名流程（joinActivity）

```mermaid
flowchart TD
    A([学生选择报名]) --> B{活动存在?}
    B -- 否 --> X1[提示: 活动不存在]
    B -- 是 --> C{已通过审核?}
    C -- 否 --> X2[提示: 尚未通过审核]
    C -- 是 --> D{已结束?}
    D -- 是 --> X3[提示: 活动已结束]
    D -- 否 --> E{已报名或已在候补?}
    E -- 是 --> X4[提示: 已报名 / 已在候补]
    E -- 否 --> F{与已报名/候补活动时间重叠?}
    F -- 是 --> X5[提示: 时间冲突, 指出冲突活动]
    F -- 否 --> G{人数已满?}
    G -- 否 --> H[加入报名名单: 报名成功]
    G -- 是 --> I[加入候补名单: 提示已候补]
    H --> J[自动保存到文件]
    I --> J
```

## 3. 取消与候补递补流程（cancelActivity）

```mermaid
flowchart TD
    A([取消报名]) --> B{活动存在且未结束?}
    B -- 否 --> X[取消失败]
    B -- 是 --> C{用户在报名名单?}
    C -- 是 --> D[移出报名名单]
    D --> E{候补名单非空?}
    E -- 是 --> F[队首候补自动递补为正式报名]
    E -- 否 --> G[结束]
    F --> G
    C -- 否 --> H{用户在候补名单?}
    H -- 是 --> I[移出候补名单]
    H -- 否 --> X
    G --> S[自动保存]
    I --> S
```

## 4. 关键算法

| 算法 | 位置 | 思路 | 复杂度 |
|---|---|---|---|
| 时间冲突检测 | `Activity::overlaps`、`ActivityManager::findConflict` | 两区间重叠当且仅当 `a.start < b.end && b.start < a.end`；时间为固定格式字符串，可直接比较；遍历该用户已报名和候补的活动逐一判断 | O(n) |
| 兴趣匹配度 | `ActivityManager::matchScore` | Jaccard：`交集 / 并集`，用 `set` 求交集 | O(m log m) |
| 推荐排序 | `ActivityManager::recommend` | 过滤未审核、已结束、已报名的活动，按匹配度降序，相同则开始时间早的在前，`stable_sort` | O(n log n) |
| 候补递补 | `promoteWaiting` | 名额空出后按候补队首依次转正，直到名额满或队列空 | O(k) |
| 时间合法性校验 | `Activity::isValidTime` | 固定格式逐位检查，再按月份天数和闰年判断日期是否存在 | O(1) |
| 积分统计 | `ActivityManager::getUserPoints` | 累加用户已报名、已审核、已结束活动的 `calcPoints()`（多态） | O(n) |

## 5. 数据文件

`data/activities.txt`：`id|clubId|type|name|tags|start|end|location|max|status|extra`

`data/registrations.txt`：`userId|activityId|registered` 或 `userId|activityId|waitlist`（同一活动内的先后顺序即候补顺序）
