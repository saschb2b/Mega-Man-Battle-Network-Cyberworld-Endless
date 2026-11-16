"""Every route of tools/cw_routes.py arrives on the screen it names: a
route that stops short (the run's opening gift came to stand before the
layer's and the battle's, unseen) sends every capture made from it to the
wrong screen."""
import cw_routes

# (BN5's Darkloids in BN6's engine: BN5's ROM beside BN6's)
MANY = {
    name: dict(START=name, STEPS=[('expect', f'screen={screen}')], NEEDS=('bn5',) if name.startswith('darkloid/') else ())
    for name, (_, _, screen) in cw_routes.ROUTES.items()
}
