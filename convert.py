# convert mdp to dat
import sys
from struct import pack, unpack

help_message = "Please give me input file name"
argc = len(sys.argv)
print(argc)
if (argc<2):
    print(help_message)
    exit(1)
filename = sys.argv[1]
if (argc >2):
    fout = sys.argv[2]
else:
    name, _ = filename.rsplit('.', 1)
    fout = name + ".dat"

print(f"{fout}") 

with open(filename, 'rt') as filein:
    lines = filein.readlines()

ind_mass = -1
mass = []
counts = [] 
countsByMass = []
timeByMass = []   
import pdb
# pdb.set_trace()
for line in lines:
    if line.startswith("total counts"):
        break
    items = line.split()
    if len(items)==2:
        if items[1]=="masses":
            num_masses = int(items[0])
        if items[1]=="cycles":
            num_cycles = int(items[0])
     
    if len(items)==3:
        if line.startswith("mass is"):
            mass.append(int(items[2]))
            ind_mass += 1
            cyc = []
            continue
    if ind_mass >= 0:
        if len(items) in (2,4,6,8):
            for i in range(len(items)//2):
                # print(items[2*i+1])
                cyc.append(int(float(items[2*i+1])))
                if ind_mass+1==num_masses:
                    timeByMass.append(int(items[2*i]))
        if len(items) == 0:
            countsByMass.append(cyc)
            
print(f"{num_masses}") 
print(f"{num_cycles}")
print(f"{mass}") 
print(f"{countsByMass}")
print(f"{timeByMass}")

offset = (5632,5632,5632,5632) # (180,180,180,180)
tm = (5,5,5,5)  # ???
hall6 = (6,6,6,6) # ???
hall3 = (3,3,3,3) # ???
counts = []
for c in range(num_cycles):
    countsByCycle=[]
    for m in range(num_masses):
        countsByCycle.append(countsByMass[m][c])
    counts.append(countsByCycle)
print(f"{counts=}")
    

#counts=((242783,13199,163,3),(233222,12152,142,4),(232163,12369,144,3),(231054,11764,133,2),
#        (231709,12977,181,1),(232364,12469,168,4),(233695,12711,145,3),(233445,13023,183,8))
#cycle_time0=(14,32,49,67,85,103,120,138)
cycle_time=tuple((18*x+1 for x in timeByMass))
with open(fout, 'wb') as fileo:
    # internal file name, 20
    fname = fout.encode()[-18:]
    rem = 20 - len(fname)
    fileo.write(fname)
    fileo.write(b'\x00'*rem)
    # internal file name over, len=20
       
    zero=b'\x00'
    # 14 unknown
    fileo.write(zero*14)
    
    # 8 sputter_time
    fileo.write(zero*8)
    
    # 24 unknown
    fileo.write(zero*24)
    
    # x1,y1,x2,y2
    fileo.write(zero*4)
    
    # 8 unknown
    fileo.write(zero*8)
    
    # 8 ipc
    fileo.write(zero*8)
    
    # 8 fpc
    fileo.write(zero*8)
    
    # 8 matrix_conc
    fileo.write(zero*8)
    
    # 5 matrix_name
    fileo.write(zero*5)
    
    # 11 unknown
    fileo.write(zero*11)
    
    # 4 start=1 b'\x01\x00\x00\x00'
    fileo.write(pack('i',1))
    
    # 2 FA
    fileo.write(zero*2)
    
    # 2 CD
    fileo.write(zero*2)
    
    # 2 Raster
    fileo.write(zero*2)
    
    # 2 turns
    fileo.write(zero*2)
    
    # 3 beam_species
    fileo.write(zero*3)
    
    # 53 unknown
    fileo.write(zero*53)
    
    # 2 image
    fileo.write(zero*2)
    
    # 2 prim,sec
    fileo.write(zero*2)
    
    # 7 imf4f
    fileo.write(b'IMF 4F'+zero)
    
    # 317 unknown
    fileo.write(zero*317)
    
    # important data begin
    # 2 num_masses
    fileo.write(pack('H', num_masses))
    
    # 16 unknown
    fileo.write(b'\x00\x16\x57\x16\xb9\x25\xe6\x00\x6f\x16\x33\x04\x33\x04\x6f\x16')
    
    # 60*num_masses
    for ind in range(num_masses):
        # 6, species
        fileo.write(b'\x00\x16\x57\x16\xb9\x25')
        # 10, unknown
        fileo.write(b'\xe6\x00\x6f\x16\x33\x04\x33\x04\x6f\x16')
        # 8, mass
        fileo.write(pack('<d', float(mass[ind])))
        # 2, offset
        fileo.write(pack('h', 5632)) # 180?  b'0016'
        # 10, unknown
        fileo.write(b'\x57\x16\xb9\x25\xe6\x20\x6f\x16\x33\x04')
        # 2, time
        fileo.write(b'\x33\x04')
        # 2, hall6
        fileo.write(b'\x6f\x16')
        # 2, hall3
        fileo.write(b'\x32\x03')
        # 18, unknown
        fileo.write(b'\x0f\x00\x00\x00\x00\x00\x00\x00\x82\xb6\x0f\x00\x48\x01\xd0\x00\x00\x00')
        
    # (4*num_masses+4)*num_cycles
    for cycle in range(num_cycles):
        for ind in range(num_masses):
            fileo.write(pack('i', counts[cycle][ind]))
        fileo.write(pack('i', cycle_time[cycle]))
