"""Regression tests for activity expiry, audit messages and rejection persistence."""
from pathlib import Path
import argparse, subprocess, tempfile, stat

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',default=str(Path(__file__).resolve().parents[1]/'campushub.exe'))
args=parser.parse_args()
exe=Path(args.exe).resolve()
root=Path(__file__).resolve().parents[1]
passed=0

def check(name,ok):
    global passed
    assert ok,name
    passed+=1
    print('PASS',name,flush=True)

with tempfile.TemporaryDirectory(prefix='.campushub-activity-review-',dir=root) as temp:
    temp=Path(temp)
    sources=[root/'tests/activity_review.cpp',*sorted((root/'src/activity').glob('*.cpp'))]
    unit=temp/'activity_review.exe'
    subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-pedantic','-finput-charset=UTF-8',
                    '-fexec-charset=UTF-8','-static',*[str(s) for s in sources],'-o',str(unit)],check=True)
    result=subprocess.run([str(unit)],cwd=temp,capture_output=True,encoding='utf-8',timeout=10)
    print(result.stdout,flush=True)
    assert result.returncode==0,result.stderr

    def fixture(name,pending=True,club_status='approved'):
        p=temp/name
        (p/'data').mkdir(parents=True)
        data={'users.txt':'A|Admin|pw|Admin|None|None|normal\nM|Manager|pw|ClubManager|CS|sport|normal\nS1|One|pw|Student|CS|sport|normal\nS2|Two|pw|Student|CS|sport|normal\n',
              'clubs.txt':f'普通社团\n\nC001\nTest Club\nintro\nM\n{club_status}\n',
              'club_members.txt':'','club_applications.txt':'','retired_user_ids.txt':'',
              'activities.txt':('3001|C001|Lecture|Expired|sport|2020-01-01 14:00|2020-01-01 16:00|room|1|pending|speaker\n' if pending else '')+
                '3002|C001|Lecture|Future|sport|2099-01-01 14:00|2099-01-01 16:00|room|5|pending|speaker\n',
              'registrations.txt':'S1|3001|registered\nS2|3001|waitlist\n' if pending else ''}
        for name,value in data.items(): (p/'data'/name).write_text(value,encoding='utf-8')
        return p

    def run(p,lines):
        result=subprocess.run([str(exe)],cwd=p,input='\n'.join(lines)+'\n',capture_output=True,
                              encoding='utf-8',timeout=10)
        assert result.returncode==0,result.stdout+result.stderr
        return result.stdout
    def text(p,name): return (p/'data'/name).read_text(encoding='utf-8')
    def snapshot(p): return {f.name:f.read_bytes() for f in (p/'data').iterdir() if f.is_file()}
    def audit(p,id,choice): return run(p,['1','A','pw','3','2',id,str(choice),'0','0','0'])

    p=fixture('expired_approve'); before=snapshot(p)
    output=audit(p,'3001',1)
    check('Expired pending is labeled and approval explains expiry',
          '已结束（仅可拒绝）' in output and '审核失败：活动已结束' in output and '已经审核过' not in output and before==snapshot(p))
    output=audit(p,'3001',2)
    check('Expired pending rejection persists and preserves registrations',
          '已拒绝该活动' in output and '|rejected|' in text(p,'activities.txt').splitlines()[0] and
          text(p,'registrations.txt')=='S1|3001|registered\nS2|3001|waitlist\n')
    output=run(p,['1','A','pw','3','2','3002','0','0','0','0'])
    check('Rejected expired activity leaves pending list after restart','Expired' not in output and 'Future' in output)

    p=fixture('cancel'); before=snapshot(p)
    check('Cancel changes no data','已取消审核' in audit(p,'3001',0) and snapshot(p)==before)
    p=fixture('save_failure'); before=snapshot(p)
    locked=p/'data/activities.txt'; locked.chmod(stat.S_IREAD)
    try: output=audit(p,'3001',2)
    finally: locked.chmod(stat.S_IREAD|stat.S_IWRITE)
    check('Rejection save failure accurately reported with rollback',
          '审核失败：文件保存失败' in output and '已拒绝该活动！' not in output and before==snapshot(p))
    check('Rejection succeeds after permissions restored','已拒绝该活动！' in audit(p,'3001',2))

    p=fixture('future')
    check('Future approval still works','活动已通过审核！' in audit(p,'3002',1) and '|approved|' in text(p,'activities.txt').splitlines()[1])
    output=audit(p,'3002',2)
    check('Already reviewed reason is accurate','审核失败：该活动不是待审核状态。' in output)
    output=audit(p,'9999',2)
    check('Missing activity reason is accurate','审核失败：活动不存在。' in output)

    p=fixture('inactive_club',club_status='dissolved')
    check('Pending activity from dissolved club can still be rejected','已拒绝该活动！' in audit(p,'3001',2))

    p=fixture('publish',pending=False); before=snapshot(p)
    output=run(p,['1','M','pw','5','4','1','1','Past','sport','2020-01-01 14:00','2020-01-01 16:00','room','5','speaker','0','0','0','0'])
    check('Interactive publishing refuses past end without writing','结束时间必须晚于当前时间' in output and '活动已提交' not in output and before==snapshot(p))
    output=run(p,['1','M','pw','5','4','2','3002','','','','','2020-01-01 14:00','2020-01-01 16:00','0','0','0','0'])
    check('Interactive modification refuses past end without writing','结束时间必须晚于当前时间' in output and before==snapshot(p))
    output=run(p,['1','A','pw','2','0','0','0'])
    check('Club menu consistently describes dissolution','3.解散违规社团（保留历史）' in output and '删除违规社团' not in output)
print('ALL UI PASSED',passed)
