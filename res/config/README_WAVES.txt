# 刷怪配置说明（Wave CSV）
# 文件格式：time_sec,zombie_id,row,start_x,count,interval_sec
#
# 字段说明：
# 1) time_sec: 开始刷怪时间（秒）
# 2) zombie_id: 僵尸ID（normal/cone/bucket/football/screen）
# 3) row: 行号（0~4），填 -1 表示随机行
# 4) start_x: 出生X坐标（建议 980~1000）
# 5) count: 本组刷怪数量（>=1）
# 6) interval_sec: 同组多只之间间隔秒数，0表示同一时刻刷出
#
# 示例：
# 0.8,normal,2,980,1,0
# 20.0,cone,-1,985,3,2.0
#
# 扩展其它关卡建议：
# 1) 复制 level1_waves.csv 为 level2_waves.csv
# 2) 在场景配置入口改为对应文件路径
