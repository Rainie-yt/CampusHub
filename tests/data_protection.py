"""Startup must reject damaged existing data without changing any data file."""
from pathlib import Path
import argparse
import ctypes
import os
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe', default=str(Path(__file__).resolve().parents[1] / 'campushub.exe'))
args = parser.parse_args()
EXE = Path(args.exe).resolve()
if not EXE.is_file():
    parser.error('Build campushub.exe first or pass --exe')

USERS = ('A|Admin|pw|Admin|None|None|normal\n'
         'M|Manager|pw|ClubManager|CS|sport|normal\n'
         'S1|Student1|pw|Student|CS|sport|normal\n'
         'S2|Student2|pw|Student|CS|sport|normal\n'
         'S3|Student3|pw|Student|CS|sport|normal\n')
CLUBS = ('普通社团\n\nC001\nTest Club\nintro\nM\napproved\n'
         '普通社团\n\nC002\nPending Club\nintro\nS3\npending\n'
         '普通社团\n\nC003\nRejected Club\nintro\nS2\nrejected\n')
ACTIVITIES = ('3001|C001|Lecture|Future|sport|2099-01-01 14:00|2099-01-01 16:00|room|2|approved|speaker\n'
              '3002|C001|Lecture|History|sport|2020-01-01 14:00|2020-01-01 16:00|room|2|approved|speaker\n')
DEFAULTS = {'users.txt': USERS, 'clubs.txt': CLUBS, 'club_members.txt': 'C001 S2\n',
            'club_applications.txt': 'C001 S3\n', 'activities.txt': ACTIVITIES,
            'registrations.txt': 'S1|3002|registered\n', 'retired_user_ids.txt': ''}
PASSED = 0


def snapshot(p):
    return {str(f.relative_to(p)): f.read_bytes() if f.is_file() else None
            for f in p.rglob('*')}


def run(p, lines):
    return subprocess.run([str(EXE)], cwd=p, input='\n'.join(lines) + '\n',
                          capture_output=True, encoding='utf-8', errors='replace', timeout=5)


def check(name, condition):
    global PASSED
    assert condition, name
    PASSED += 1
    print('PASS', name, flush=True)


with tempfile.TemporaryDirectory(prefix='.campushub-data-protection-',
                                 dir=Path(__file__).resolve().parents[1]) as temp:
    root = Path(temp)

    def fixture(name):
        p = root / name
        (p / 'data').mkdir(parents=True)
        for filename, value in DEFAULTS.items():
            (p / 'data' / filename).write_text(value, encoding='utf-8')
        return p

    def rejects(name, filename, value, line):
        p = fixture(name)
        f = p / 'data' / filename
        f.write_text(value, encoding='utf-8')
        before = snapshot(p)
        # Supply a real write operation: it must never reach the menu.
        r = run(p, ['1', 'S1', 'pw', '2', '4', 'C001', '0', '0', '0'])
        output = r.stdout + r.stderr
        check(name, r.returncode != 0 and '启动已取消' in output
              and filename in output and f'第 {line} 行' in output
              and '请选择' not in output and snapshot(p) == before)

    rejects('clubs_incomplete', 'clubs.txt', CLUBS + '普通社团\n', 22)
    rejects('clubs_bad_type', 'clubs.txt', CLUBS.replace('普通社团', 'unknown', 1), 1)
    rejects('clubs_duplicate', 'clubs.txt', CLUBS.replace('C002', 'C001'), 8)
    rejects('clubs_bad_status', 'clubs.txt', CLUBS.replace('approved', 'bad', 1), 7)
    rejects('member_unknown_club', 'club_members.txt', 'C001 S2\nC999 S1\n', 2)
    rejects('member_incomplete', 'club_members.txt', 'C001 S2\nBROKEN\n', 2)
    rejects('member_extra_field', 'club_members.txt', 'C001 S2 extra\n', 1)
    rejects('member_duplicate', 'club_members.txt', 'C001 S2\nC001 S2\n', 2)
    rejects('application_unknown_club', 'club_applications.txt', 'C001 S3\nC999 S1\n', 2)
    rejects('application_incomplete', 'club_applications.txt', 'C001\n', 1)
    rejects('application_duplicate', 'club_applications.txt', 'C001 S3\nC001 S3\n', 2)
    rejects('application_existing_member', 'club_applications.txt', 'C001 S2\n', 1)
    rejects('activity_missing_field', 'activities.txt', ACTIVITIES + '3003|C001|Lecture\n', 3)
    rejects('activity_duplicate', 'activities.txt', ACTIVITIES.replace('3002|', '3001|'), 2)
    rejects('activity_invalid_time', 'activities.txt', ACTIVITIES.replace('2099-01-01 14:00', 'bad'), 1)
    rejects('registration_unknown_activity', 'registrations.txt', 'S1|3002|registered\nS2|9999|registered\n', 2)
    rejects('registration_bad_state', 'registrations.txt', 'S1|3002|bad\n', 1)
    rejects('registration_duplicate', 'registrations.txt', 'S1|3002|registered\nS1|3002|registered\n', 2)
    rejects('user_missing_field', 'users.txt', USERS + 'broken\n', 6)
    rejects('user_duplicate', 'users.txt', USERS + USERS.splitlines()[0] + '\n', 6)
    rejects('user_bad_role', 'users.txt', USERS.replace('|Admin|None|', '|bad|None|'), 1)
    rejects('nul_byte', 'activities.txt', ACTIVITIES + '\0\n', 3)

    for filename in DEFAULTS:
        p = fixture('directory_' + filename)
        f = p / 'data' / filename
        f.unlink()
        f.mkdir()
        before = snapshot(p)
        r = run(p, ['0'])
        output = r.stdout + r.stderr
        check('directory_' + filename, r.returncode != 0 and '启动已取消' in output
              and '请选择' not in output and snapshot(p) == before
              and (filename in output or filename == 'retired_user_ids.txt'))

    if os.name == 'nt':
        from ctypes import wintypes
        kernel = ctypes.WinDLL('kernel32', use_last_error=True)
        kernel.CreateFileW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD,
                                      ctypes.c_void_p, wintypes.DWORD, wintypes.DWORD, wintypes.HANDLE]
        kernel.CreateFileW.restype = wintypes.HANDLE
        kernel.CloseHandle.argtypes = [wintypes.HANDLE]
        kernel.CloseHandle.restype = wintypes.BOOL
        for filename in DEFAULTS:
            p = fixture('locked_' + filename)
            before = snapshot(p)
            handle = kernel.CreateFileW(str(p / 'data' / filename), 0x80000000, 0, None, 3, 128, None)
            if handle == ctypes.c_void_p(-1).value:
                raise ctypes.WinError(ctypes.get_last_error())
            try:
                r = run(p, ['0'])
            finally:
                kernel.CloseHandle(handle)
            output = r.stdout + r.stderr
            check('locked_' + filename, r.returncode != 0 and '启动已取消' in output
                  and '请选择' not in output and snapshot(p) == before)

    p = fixture('blank_club_separators')
    (p / 'data/clubs.txt').write_text('\n\n' + CLUBS.replace('pending\n', 'pending\n\n') + '\n \t\n', encoding='utf-8')
    r = run(p, ['1', 'S1', 'pw', '2', '8', '4', 'New Club', 'intro', '0', '0', '0'])
    saved = (p / 'data/clubs.txt').read_text(encoding='utf-8')
    check('blank_lines_keep_clubs_and_next_id', r.returncode == 0 and 'C004' in r.stdout
          and all(f'C00{i}\n' in saved for i in range(1, 5))
          and (p / 'data/club_members.txt').read_text(encoding='utf-8') == DEFAULTS['club_members.txt'])

    p = fixture('crlf_and_bom')
    for filename in ('clubs.txt', 'club_members.txt', 'club_applications.txt', 'activities.txt', 'registrations.txt'):
        (p / 'data' / filename).write_bytes(b'\xef\xbb\xbf' + DEFAULTS[filename].replace('\n', '\r\n').encode('utf-8'))
    before = snapshot(p)
    r = run(p, ['0'])
    check('crlf_and_bom_no_rewrite', r.returncode == 0 and snapshot(p) == before)

    p = fixture('old_six_field_users')
    (p / 'data/users.txt').write_text(USERS.replace('|normal', ''), encoding='utf-8')
    r = run(p, ['1', 'S1', 'pw', '0', '0'])
    check('legacy_users_still_login', r.returncode == 0 and '登录成功' in r.stdout)

    p = fixture('first_run')
    for filename in DEFAULTS:
        (p / 'data' / filename).unlink()
    before = snapshot(p)
    r = run(p, ['0'])
    check('all_missing_files_start_without_writes', r.returncode == 0 and snapshot(p) == before)
    r = run(p, ['2', 'NEW', 'New User', 'pw', 'CS', 'sport', '0'])
    check('first_registration_creates_users', r.returncode == 0 and '注册成功' in r.stdout
          and 'NEW|' in (p / 'data/users.txt').read_text(encoding='utf-8'))

    p = fixture('missing_members_and_applications')
    for filename in ('club_members.txt', 'club_applications.txt'):
        (p / 'data' / filename).unlink()
    r = run(p, ['0'])
    check('missing_optional_relationship_files', r.returncode == 0)

    p = fixture('missing_activity_with_existing_registration')
    (p / 'data/activities.txt').unlink()
    before = snapshot(p)
    r = run(p, ['0'])
    check('missing_activity_does_not_discard_registration', r.returncode != 0
          and 'registrations.txt' in r.stderr and snapshot(p) == before)

    p = fixture('error_before_retired_id_migration')
    (p / 'data/users.txt').write_text(USERS.replace('S1|Student1|pw|Student|CS|sport|normal\n', ''), encoding='utf-8')
    (p / 'data/retired_user_ids.txt').unlink()
    (p / 'data/club_members.txt').write_text('C999 S2\n', encoding='utf-8')
    before = snapshot(p)
    r = run(p, ['0'])
    check('failed_startup_never_writes_retired_ids', r.returncode != 0 and snapshot(p) == before)

print('ALL PASSED', PASSED)
