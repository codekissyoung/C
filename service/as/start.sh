#
# 本脚本文件, 除了在shell中人为执行以外, 可能被系统crontab任务自动执行;
# 由于crontab执行时, 环境变量的读取和shell自动读取存在不同, 所以在执行
# 之前拓展系统的 $LD_LIBRARY_PATH 变量, 保证能够在当前目录下加载成功动态库
# 文件. 这些动态库文件没有在配置文件中设置路径的.
#
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
./Aggregator_Service.Exe -c Aggregator_Service.ini -d start
