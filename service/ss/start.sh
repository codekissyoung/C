#
# ���ű��ļ�, ������shell����Ϊִ������, ���ܱ�ϵͳcrontab�����Զ�ִ��;
# ����crontabִ��ʱ, ���������Ķ�ȡ��shell�Զ���ȡ���ڲ�ͬ, ������ִ��
# ֮ǰ��չϵͳ�� $LD_LIBRARY_PATH ����, ��֤�ܹ��ڵ�ǰĿ¼�¼��سɹ���̬��
# �ļ�. ��Щ��̬���ļ�û���������ļ�������·����.
#
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
./Suggestion_Service.Exe -c Suggestion_Service.ini -d start
