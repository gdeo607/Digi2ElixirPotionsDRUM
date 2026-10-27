import ctypes, struct, sys, wave, math
lib = ctypes.CDLL(sys.argv[1]); out_dir = sys.argv[2]
F = "pitch tune model wave sweep sdec mod velocity active released freq clvl click mod2 frq2 cpit".split()
class Frame(ctypes.Structure):
    _fields_ = [(n, ctypes.c_int if n in ("active","released") else ctypes.c_float) for n in F]
U = lambda x: x*0x7f00
L = lambda i: i*256.0
BASE = dict(tune=0x4000, velocity=100*256, active=1, clvl=0x4000, cpit=0x4000, freq=L(48), frq2=L(48), sdec=U(.3))
def render(track, p, secs=0.7, decay=0.18):
    q = dict(BASE); q.update(p); f = Frame(**q); out = (ctypes.c_float*32)(); res = []
    for b in range(int(48000*secs/32)):
        lib.lxm_track(track, ctypes.byref(f), 1 if b == 0 else 0, out, 32, 48000); res += list(out)
    return [x*math.exp(-i/(48000*decay)) for i,x in enumerate(res)]
def wav(path, s):
    with wave.open(path, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(48000)
        w.writeframes(b"".join(struct.pack("<h", max(-32767, min(32767, int(x*32767)))) for x in s))
P = {
 "drum_kick":    (dict(model=L(0), pitch=36*256, wave=L(0), sweep=U(.55), sdec=U(.25), click=L(1)), .25),
 "drum_fmtom":   (dict(model=L(0), pitch=50*256, wave=L(0), sweep=U(.35), sdec=U(.35), mod=U(.6), freq=L(60), click=L(2)), .3),
 "drum_zap":     (dict(model=L(0), pitch=48*256, wave=L(2), sweep=U(.9), sdec=U(.4)), .3),
 "snare_808":    (dict(model=L(1), pitch=50*256, wave=L(0), sweep=U(.3), sdec=U(.15), mod=U(.55), freq=U(.7), mod2=U(.3), frq2=U(1), click=L(1)), .18),
 "snare_noisy":  (dict(model=L(1), pitch=55*256, wave=L(1), sweep=U(.2), sdec=U(.1), mod=U(.8), freq=U(.8), mod2=U(.1), frq2=U(.85), click=L(3)), .2),
 "cymbal":       (dict(model=L(2), pitch=60*256, wave=L(3), mod=U(.6), freq=L(48+7), mod2=U(.5), frq2=L(48+17), click=L(4)), .8),
 "cymbal_bell":  (dict(model=L(2), pitch=72*256, wave=L(0), mod=U(.3), freq=L(48+15), mod2=U(.2), frq2=L(48+26)), .6),
 "hihat_closed": (dict(model=L(3), pitch=72*256, wave=L(3), mod=U(.7), freq=L(48+5), mod2=U(.6), frq2=L(48+19), click=L(4)), .05),
 "hihat_open":   (dict(model=L(3), pitch=72*256, wave=L(3), mod=U(.7), freq=L(48+5), mod2=U(.6), frq2=L(48+19), click=L(4)), .4),
}
alls=[]
for t,(name,(p,dec)) in enumerate(P.items()):
    s = render(t % 16, p, decay=dec)
    pk = max(abs(x) for x in s); rms = math.sqrt(sum(x*x for x in s)/len(s))
    print(f"{name:13s} peak {pk:.3f} rms {rms:.3f} nan {any(x!=x for x in s)}")
    wav(f"{out_dir}/{name}.wav", s); alls += s + [0.0]*4800
wav(f"{out_dir}/lxr_all.wav", alls)
