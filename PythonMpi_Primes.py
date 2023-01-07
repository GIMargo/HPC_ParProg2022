from mpi4py import MPI
from math import sqrt
import sys

def test(N, primes, filename):
    etalon = set()
    res = set()
    isErr = False
    for p in primes:
        if p in res:
            isErr = True
        else:
            res.add(int(p))

    with open(filename, 'r') as f:
        for line in f:
            for x in line.split():
                if int(x) <= N:
                    etalon.add(int(x))
    if isErr == False and etalon == res:
        return True
    else:
        return False


def findD(number_of_processes):
	d = 1
	while (d * 2 < number_of_processes):
		d *= 2
	return int(d)

def saveListToFile(N, l, filename):
    with open(filename, 'w') as f:
        f.write(str(N)+"\n")
        for i in l:
            f.write(str(i)+" ")

def findAmount(N, k):
    m = 0 # amount of numbers, considered by all processes
    if (int(sqrt(N)) % 2 == 0) and (N % 2 != 0): # корень из N чётный, а само число нет
        m = (N - int(sqrt(N))) / 2 + 1
    else:
        m = (N - int(sqrt(N))) / 2

    amount = 0 # amount of numbers, considered by one process
    if m % k == 0:
        amount = int(m / k)
    else:
        amount = int(m / k + 1)
    return amount

def test_mode():
    comm = MPI.COMM_WORLD
    k = comm.Get_size()
    pid = comm.Get_rank()
    Ns = list([2, 105, 239, 543, 1000, 5468, 7777, 10000])
    test_num = 0
    wrong_num = 0
    for N in Ns:
        if pid == 0:
            test_num += 1
            print("\n Test # "+ str(test_num)+"! N = "+ str(N)+", k = "+str(k)+"\n")
        amount = findAmount(N, k) # amount of numbers, considered by one process
        primes = [2]
        a = int(sqrt(N) + 1)
        for x in range(3, a, 2): # from 3 to sqrt(N) considering all numbers (with doubled step)	
            isPrime = 1
            for i in primes:
                if (x % i == 0): # the number is definitely not prime
                    isPrime = 0
            if isPrime:
                primes.append(int(x)) # add prime number to "primes"
        if a % 2 == 0:  # if it's not odd, finding the nearest odd
            a += 1
        # Parralel part
        if amount:
            buff = list()
            a += pid * 2 # starting from the starting point
            #print("\n Process # "+ str(pid) + ", start = " + str(a) + "\n")
            while a <= N:
                isPrime = 1
                for i in primes:
                    #print("\n i = " + str(i)+ ", a = " + str(a) + "\n")
                    if i > sqrt(a):
                        break
                    if a % i == 0:
                        isPrime = 0
                        #print("\n"+ str(a)+ " is not prime! isPrime = " + str(isPrime)+ "\n")
                        break
                if isPrime == 1:
                    #print("\n Putting "+ str(a)+ " to buff! isPrime = " + str(isPrime)+ "\n")
                    buff.append(int(a)) # putting prime to the process buff 
                a += k * 2 # we shift by a doubled step, as we go by odd
            #print("\n Process # "+ str(pid) + ", buff: \n")
            #print(buff)
            # Result making
            d = findD(k)
            #print("\n Process # "+ str(pid) + ", d = " + str(d) + "\n")
            curr_k = k
            while d > 0:
                if pid - d >= 0: # sending data
                    #print("Process # "+ str(pid) + " is ready to sent data to process # "+str(pid-d)+"\n")
                    comm.send(buff, dest=pid -  d, tag=1)
                    #print("Process # "+ str(pid) + " sent data to process # "+str(pid-d)+"\n")
                    break # now this process is done
                elif pid + d < curr_k:
                    #print("Process # "+ str(pid) + " is ready to receive data from process # "+str(pid+d)+"\n")
                    tmp = comm.recv(source=pid + d, tag=1) # receiving data
                    #print("Process # "+ str(pid) + " received data from process # "+str(pid+d)+"\n")
                    buff = buff + tmp
                    buff.sort()
                    #print(buff)
                curr_k = d
                d = d // 2
            # only master (0) process will reach this part of code
            if pid == 0:
                primes += buff
                primes.sort()
                if test(N, primes, "etalon.txt"):
                    print("\n Everything is OK! \n")
                else:
                    print("\n The result is wrong! \n")
                    wrong_num += 1
            comm.Barrier()
    if pid == 0:
        if wrong_num == 0:
            print("\n All "+str(test_num)+" tests passed succesfully! \n")
        else:
            print("\n "+str(test_num - wrong_num)+" of "+str(test_num)+ " tests passed succesfully! \n "+ str(wrong_num)+ " tests failed! \n")


def interactive_mode(N, isExperiment):
    comm = MPI.COMM_WORLD
    k = comm.Get_size()
    pid = comm.Get_rank()
    #print("\nHello from process #"+str(pid)+" of "+str(k)+"\n")
    amount = findAmount(N, k) # amount of numbers, considered by one process
    #print("\n Process # "+ str(pid) + ", amount = " + str(amount) + "\n")
    primes = [2]
    a = int(sqrt(N) + 1)
    for x in range(3, a, 2): # from 3 to sqrt(N) considering all numbers (with doubled step)	
        isPrime = 1
        for i in primes:
            if (x % i == 0): # the number is definitely not prime
                isPrime = 0
        if isPrime:
            primes.append(int(x)) # add prime number to "primes"
    if a % 2 == 0:  # if it's not odd, finding the nearest odd
        a += 1	
    #print(primes)	
    # Parralel part
    if amount:
        buff = list()
        a += pid * 2 # starting from the starting point
        #print("\n Process # "+ str(pid) + ", start = " + str(a) + "\n")
        while a <= N:
            isPrime = 1
            for i in primes:
                #print("\n i = " + str(i)+ ", a = " + str(a) + "\n")
                if i > sqrt(a):
                    break
                if a % i == 0:
                    isPrime = 0
                    #print("\n"+ str(a)+ " is not prime! isPrime = " + str(isPrime)+ "\n")
                    break
            if isPrime == 1:
                #print("\n Putting "+ str(a)+ " to buff! isPrime = " + str(isPrime)+ "\n")
                buff.append(int(a)) # putting prime to the process buff 
            a += k * 2 # we shift by a doubled step, as we go by odd
        #print("\n Process # "+ str(pid) + ", buff: \n")
        #print(buff)
        # Result making
        d = findD(k)
        #print("\n Process # "+ str(pid) + ", d = " + str(d) + "\n")
        curr_k = k
        while d > 0:
            if pid - d >= 0: # sending data
                #print("Process # "+ str(pid) + " is ready to sent data to process # "+str(pid-d)+"\n")
                comm.send(buff, dest=pid -  d, tag=1)
                #print("Process # "+ str(pid) + " sent data to process # "+str(pid-d)+"\n")
                return # now this process is done
            elif pid + d < curr_k:
                #print("Process # "+ str(pid) + " is ready to receive data from process # "+str(pid+d)+"\n")
                tmp = comm.recv(source=pid + d, tag=1) # receiving data
                #print("Process # "+ str(pid) + " received data from process # "+str(pid+d)+"\n")
                buff = buff + tmp
                #buff.sort()
                #print(buff)
            curr_k = d
            d = d // 2
        # only master (0) process will reach this part of code
        primes += buff
        primes.sort()
        #print(primes)
        if isExperiment == False:
            saveListToFile(N, primes, "result.txt")

comm = MPI.COMM_WORLD
k = comm.Get_size()
pid = comm.Get_rank()
user_input = None
if pid == 0:
    user_input = sys.stdin.read().split()

user_input = comm.bcast(user_input, root=0)

if user_input[0] == "i":
   #print("\nInteractive mode!\n")
    interactive_mode(int(user_input[1]), False)
elif user_input[0] == "t":
    #print("\nTest mode!\n")
    test_mode()
elif user_input[0] == "e":
    #print("\nExperiment mode!\n")
    interactive_mode(int(user_input[1]), True)
else:
    print("\n Error! No such mode:"+user_input[0]+"\n")



