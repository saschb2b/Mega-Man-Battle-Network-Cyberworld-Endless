"""Each game's start (issue 865; docs/game/meta/forms.md, Each game's start):
with FORM on SOUL BN5 the Standard folder is BN5's own NEW GAME folder,
on SOUL BN4 BN4's (AirShot in *, Vulcan1 in S, the setup's line naming
both), read from their ROMs as the run starts; CROSS keeps BN6's own. The
setup says so, the run's first Custom screen draws from that folder (the
state's folder line, its sum each start's), and a quit and CONTINUE keep
it. The sums: BN6's Standard 6eff, BN5's start 13b5, BN4's 299a."""

SUM = {'CROSS': '6eff', 'SOUL BN5': '13b5', 'SOUL BN4': '299a'}
LINE = {'CROSS': "BN6's own starting folder", 'SOUL BN5': "BN5's own starting folder",
        'SOUL BN4': "BN4's own starting folder. AirShot in *, Vulcan1 in S"}
NEEDS = {'CROSS': (), 'SOUL BN5': ('bn5',), 'SOUL BN4': ('bn5', 'bn4')}
FORM = {'CROSS': 1, 'SOUL BN5': 2, 'SOUL BN4': 3}


def setup(form):
    """The setup with every folder and form open: FORM turned to `form`, the
    Folder row's line its start's, JACK IN!, the run's folder in Lan's room"""
    return [
        ('expect', 'startfolder=Standard'),
        ('repeat', 'press DOWN; wait 6', 'row=form', 8),
        ('repeat', 'press RIGHT; wait 6', f'form={form}', 4),
        ('expect', f'folderline={LINE[form]}'),
        ('repeat', 'press UP; wait 6', 'row=folder', 8),
        ('expect', f'folderline={LINE[form]}'),
        ('repeat', 'press START; wait 60', 'scene=emu'),
        ('until', 'where=town', 600),
        ('expect', f'runform={form}'),
        ('expect', f'folder~sum {SUM[form]}'),
    ]


def battle(form):
    """A run begun in the net in `form`: its first Custom screen's folder,
    then the game quit and CONTINUEd, the folder kept"""
    return [
        ('expect', f'runform={form}'),
        ('until', 'screen=map', 1200),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('do', 'foes 01@52'),
        ('until', 'phase=custom', 1200),
        ('expect', f'folder~sum {SUM[form]}'),
        ('restart', '--scene title --dev quiet'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 1200),
        ('expect', f'runform={form}'),
        ('expect', f'folder~sum {SUM[form]}'),
    ]


MANY = {}
for form in SUM:
    slug = form.lower().replace(' ', '-')
    MANY[f'setup-{slug}'] = dict(START='--scene setup --dev folders,forms', NEEDS=NEEDS[form], STEPS=setup(form))
    MANY[f'battle-{slug}'] = dict(START=f'--scene emu --seed 3 --dev quiet,form={FORM[form]}', NEEDS=NEEDS[form], STEPS=battle(form))
