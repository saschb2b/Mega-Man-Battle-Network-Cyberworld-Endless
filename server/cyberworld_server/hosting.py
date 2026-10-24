"""What a host needs to know as the server starts (server/README.md, Host
your own Net Square): the configuration it read, the addresses players give
the gate's keeper, read from this computer's own interfaces, and the lines
that say them on the console.

Nothing here sends a packet. The default route's address is read from a UDP
socket connected to a documentation address (RFC 5737's TEST-NET-1), which
connect() alone never sends to; the rest are this computer's name's own
addresses."""

import contextlib
import ipaddress
from pathlib import Path
import socket
import sys

CONFIG = "server.ini"
ENVIRONMENT = "CYBERWORLD_SERVER_"   # CYBERWORLD_SERVER_SEED=7: the INI's seed
PROBE = "192.0.2.1"
LOOPBACK = {"127.0.0.1", "localhost", "::1"}
SHOWN = 4   # addresses at most: a computer with many adapters names its likeliest


def program_folder():
    """Where the server's program is: the native build's folder, or the
    zipapp's (or main.py's) own."""
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(sys.argv[0]).resolve().parent


def find_config(folders):
    """The first server.ini of `folders`, None where none has one."""
    for folder in folders:
        path = Path(folder) / CONFIG
        if path.is_file():
            return path
    return None


def environment_settings(environ):
    """The [server] keys the environment sets, CYBERWORLD_SERVER_NAME as
    name; '%' kept as typed (the INI's interpolation would read it)."""
    return {key[len(ENVIRONMENT):].lower(): value.replace("%", "%%")
            for key, value in environ.items() if key.startswith(ENVIRONMENT) and len(key) > len(ENVIRONMENT)}


def usable(address):
    """Whether another computer could reach this one at `address`."""
    try:
        ip = ipaddress.ip_address(address)
    except ValueError:
        return False
    return not (ip.is_loopback or ip.is_link_local or ip.is_unspecified or ip.is_multicast)


def route_address():
    """The address this computer's default route leaves from, None offline."""
    with contextlib.suppress(OSError), socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as probe:
        probe.connect((PROBE, 9))
        return probe.getsockname()[0]
    return None


def local_addresses(named=True):
    """This computer's IPv4 addresses others can reach it at, the default
    route's first. `named` adds its name's addresses, which can wait on a
    name service: called off the event loop."""
    found = [route_address()]
    if named:
        with contextlib.suppress(OSError, UnicodeError):
            found += [entry[4][0] for entry in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET)]
    addresses = []
    for address in found:
        if address and usable(address) and address not in addresses:
            addresses.append(address)
    return addresses[:SHOWN]


def contained():
    """Whether the server runs in a container (Docker's or Podman's mark),
    where its own addresses are the container's, not the host's."""
    return Path("/.dockerenv").exists() or Path("/run/.containerenv").exists()


def banner(name, host, port, secure, addresses, state_dir, config=None, container=False, windows=False):
    """The lines the console shows as the Square opens."""
    scheme = "wss" if secure else "ws"
    lines = [f'The Net Square "{name}" is open on port {port}.']
    if host in LOOPBACK:
        lines += ["Only games on this computer can come in. They give the gate's keeper:",
                  f"  {scheme}://localhost:{port}",
                  "(host = 127.0.0.1: leave host empty in server.ini to let other computers in.)"]
    elif secure:
        lines += ["Players give the Mr.Prog at the gate in Lan's HP the domain its certificate names:",
                  f"  {scheme}://YOUR-DOMAIN:{port}"]
    elif container:
        lines += ["In a container: players give the Mr.Prog at the gate in Lan's HP the address",
                  f"of the computer it runs on, with the port published for it ({scheme}://HOST:PORT),",
                  "or the domain a proxy in front of it serves."]
    else:
        shown = [host] if host and host not in ("0.0.0.0", "::") else addresses
        if shown:
            lines.append("Players give the Mr.Prog at the gate in Lan's HP, on this network:")
            lines += [f"  {scheme}://{address}:{port}" for address in shown]
        else:
            lines.append("This computer has no network now: players reach it at its address, port " + str(port) + ".")
        lines.append(f"  {scheme}://localhost:{port}  (a game on this computer)")
        lines += [f"From the internet: forward TCP port {port} on the router to this computer and give",
                  f"{scheme}://YOUR-PUBLIC-ADDRESS:{port}, or see server/README.md, Host your own Net Square."]
        if windows:
            lines.append("Windows may ask whether to let it through the firewall: allow it on private networks.")
    lines.append(f"Settings: {config}" if config else "Settings: the defaults (server.ini beside the server changes them).")
    lines.append(f"Its state, to keep across updates: {Path(state_dir).resolve()}")
    lines.append("Stopping the container closes the Square." if container else "Ctrl+C closes the Square.")
    return lines


def announce(lines, stream=None):
    stream = stream or sys.stdout
    stream.write("\n".join(lines) + "\n")
    stream.flush()
