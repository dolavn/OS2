import subprocess
for i in range(50): 
    print("Running " + str(i) + " iteration")
    subprocess.check_output(['make','clean','qemu'])
