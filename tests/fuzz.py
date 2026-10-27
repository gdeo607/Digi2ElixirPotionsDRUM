import ctypes, sys, random, math
lib = ctypes.CDLL(sys.argv[1])
F = "pitch tune model wave sweep sdec mod velocity active released freq clvl click mod2 frq2 cpit".split()
class Frame(ctypes.Structure):
    _fields_ = [(n, ctypes.c_int if n in ("active","released") else ctypes.c_float) for n in F]
out=(ctypes.c_float*32)(); bad=0; worst=0
random.seed(int(sys.argv[2]) if len(sys.argv)>2 else 1)
# targeted: each model, FREQ swept 0..127
for model in range(4):
    for fq in range(0,128,8):
        f=Frame(pitch=60*256,tune=0x4000,model=model*256,wave=0,sweep=0x4000,sdec=0x3000,mod=0x5000,velocity=100*256,active=1,freq=fq*256,clvl=0x4000,click=256,mod2=0,frq2=0x3000,cpit=0x4000)
        for b in range(60):
            lib.lxm_track(model,ctypes.byref(f),1 if b==0 else 0,out,32,48000)
            m=max(abs(x) if x==x else 1e9 for x in out); worst=max(worst,m)
            if m>4: bad+=1; print('BAD model',model,'freq',fq,'block',b,m); break
# random
for it in range(3000):
    vals={n:random.random()*0x7f00 for n in F if n not in ("active","released")}
    vals['model']=random.randrange(4)*256.0; vals['wave']=random.randrange(7)*256.0; vals['click']=random.randrange(5)*256.0
    vals['pitch']=random.randrange(0,128)*256.0
    f=Frame(active=1,**vals); t=it%16
    for b in range(4):
        lib.lxm_track(t,ctypes.byref(f),1 if (b==0 and random.random()<.5) else 0,out,32,48000)
        m=max(abs(x) if x==x else 1e9 for x in out); worst=max(worst,m)
        if m>4: bad+=1; print('BAD random',{k:int(v) for k,v in vals.items()},m); break
    if bad>5: break
print('bad',bad,'worst peak',worst)
