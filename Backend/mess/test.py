import numpy as np, sys, os
import pandas as pd

def verify_student(rollNumber):
    hostel = np.loadtxt(os.path.join(sys.path[0] + "\H5Students.csv"), delimiter=",", dtype=str)[None].T
    zero = np.zeros(hostel.shape[0], dtype=int)
    print(hostel.shape)
    print(zero.shape)
    new = np.concatenate([hostel, zero], axis=1)
    print(new)
    DF = pd.DataFrame(new)
    DF.to_csv(sys.path[0] + "\H5Students.csv", index=False, header=False)
    if str(rollNumber) in hostel:
        return True
    else:
        return False

print(verify_student("200070016"))
