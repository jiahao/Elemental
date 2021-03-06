#
#  Copyright (c) 2009-2015, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
from El.core import *
from solvers import *

from ctypes import CFUNCTYPE

# Basis pursuit
# =============
lib.ElBP_s.argtypes = \
lib.ElBP_d.argtypes = \
lib.ElBPDist_s.argtypes = \
lib.ElBPDist_d.argtypes = \
lib.ElBPSparse_s.argtypes = \
lib.ElBPSparse_d.argtypes = \
lib.ElBPDistSparse_s.argtypes = \
lib.ElBPDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p]

lib.ElBPX_s.argtypes = \
lib.ElBPXDist_s.argtypes = \
lib.ElBPXSparse_s.argtypes = \
lib.ElBPXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPDirectCtrl_s]
lib.ElBPX_d.argtypes = \
lib.ElBPXDist_d.argtypes = \
lib.ElBPXSparse_d.argtypes = \
lib.ElBPXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPDirectCtrl_d]

def BP(A,b,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBP_s(*args)
      else:            lib.ElBPX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBP_d(*args)
      else:            lib.ElBPX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDist_s(*args)
      else:            lib.ElBPXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDist_d(*args)
      else:            lib.ElBPXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPSparse_s(*args)
      else:            lib.ElBPXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPSparse_d(*args)
      else:            lib.ElBPXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDistSparse_s(*args)
      else:            lib.ElBPXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDistSparse_d(*args)
      else:            lib.ElBPXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# ADMM
# ----
lib.ElBPADMM_s.argtypes = \
lib.ElBPADMM_d.argtypes = \
lib.ElBPADMM_c.argtypes = \
lib.ElBPADMM_z.argtypes = \
lib.ElBPADMMDist_s.argtypes = \
lib.ElBPADMMDist_d.argtypes = \
lib.ElBPADMMDist_c.argtypes = \
lib.ElBPADMMDist_z.argtypes = \
  [c_void_p,c_void_p,c_void_p,POINTER(iType)]
def BPADMM(A,b):
  if type(A) is not type(b):
    raise Exception('Types of A and b must match')
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  numIts = iType()
  if type(A) is Matrix:
    z = Matrix(A.tag)
    args = [A.obj,b.obj,z.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElBPADMM_s(*args)
    elif A.tag == dTag: lib.ElBPADMM_d(*args)
    elif A.tag == cTag: lib.ElBPADMM_c(*args)
    elif A.tag == zTag: lib.ElBPADMM_z(*args)
    else: DataExcept()
    return z, numIts
  elif type(A) is DistMatrix:
    z = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,z.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElBPADMMDist_s(*args)
    elif A.tag == dTag: lib.ElBPADMMDist_d(*args)
    elif A.tag == cTag: lib.ElBPADMMDist_c(*args)
    elif A.tag == zTag: lib.ElBPADMMDist_z(*args)
    else: DataExcept()
    return z, numIts
  else: TypeExcept()

# Chebyshev point
# ===============
lib.ElCP_s.argtypes = \
lib.ElCP_d.argtypes = \
lib.ElCPDist_s.argtypes = \
lib.ElCPDist_d.argtypes = \
lib.ElCPSparse_s.argtypes = \
lib.ElCPSparse_d.argtypes = \
lib.ElCPDistSparse_s.argtypes = \
lib.ElCPDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p]

lib.ElCPX_s.argtypes = \
lib.ElCPXDist_s.argtypes = \
lib.ElCPXSparse_s.argtypes = \
lib.ElCPXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPAffineCtrl_s]
lib.ElCPX_d.argtypes = \
lib.ElCPXDist_d.argtypes = \
lib.ElCPXSparse_d.argtypes = \
lib.ElCPXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPAffineCtrl_d]

def CP(A,b,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElCP_s(*args)
      else:            lib.ElCPX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElCP_d(*args)
      else:            lib.ElCPX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElCPDist_s(*args)
      else:            lib.ElCPXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElCPDist_d(*args)
      else:            lib.ElCPXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElCPSparse_s(*args)
      else:            lib.ElCPXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElCPSparse_d(*args)
      else:            lib.ElCPXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElCPDistSparse_s(*args)
      else:            lib.ElCPXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElCPDistSparse_d(*args)
      else:            lib.ElCPXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# Dantzig selector
# ================
lib.ElDS_s.argtypes = \
lib.ElDSDist_s.argtypes = \
lib.ElDSSparse_s.argtypes = \
lib.ElDSDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p]
lib.ElDS_d.argtypes = \
lib.ElDSDist_d.argtypes = \
lib.ElDSSparse_d.argtypes = \
lib.ElDSDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p]

lib.ElDSX_s.argtypes = \
lib.ElDSXDist_s.argtypes = \
lib.ElDSXSparse_s.argtypes = \
lib.ElDSXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p,
   LPAffineCtrl_s]
lib.ElDSX_d.argtypes = \
lib.ElDSXDist_d.argtypes = \
lib.ElDSXSparse_d.argtypes = \
lib.ElDSXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p,
   LPAffineCtrl_d]

def DS(A,b,lambdaPre,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  lambd = TagToType(A.tag)(lambdaPre)
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElDS_s(*args)
      else:            lib.ElDSX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElDS_d(*args)
      else:            lib.ElDSX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElDSDist_s(*args)
      else:            lib.ElDSXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElDSDist_d(*args)
      else:            lib.ElDSXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElDSSparse_s(*args)
      else:            lib.ElDSXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElDSSparse_d(*args)
      else:            lib.ElDSXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElDSDistSparse_s(*args)
      else:            lib.ElDSXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElDSDistSparse_d(*args)
      else:            lib.ElDSXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# Least Absolute Value regression
# ===============================
lib.ElLAV_s.argtypes = \
lib.ElLAV_d.argtypes = \
lib.ElLAVDist_s.argtypes = \
lib.ElLAVDist_d.argtypes = \
lib.ElLAVSparse_s.argtypes = \
lib.ElLAVSparse_d.argtypes = \
lib.ElLAVDistSparse_s.argtypes = \
lib.ElLAVDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p]

lib.ElLAVX_s.argtypes = \
lib.ElLAVXDist_s.argtypes = \
lib.ElLAVXSparse_s.argtypes = \
lib.ElLAVXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPAffineCtrl_s]
lib.ElLAVX_d.argtypes = \
lib.ElLAVXDist_d.argtypes = \
lib.ElLAVXSparse_d.argtypes = \
lib.ElLAVXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,
   LPAffineCtrl_d]

def LAV(A,b,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElLAV_s(*args)
      else:            lib.ElLAVX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElLAV_d(*args)
      else:            lib.ElLAVX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElLAVDist_s(*args)
      else:            lib.ElLAVXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElLAVDist_d(*args)
      else:            lib.ElLAVXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElLAVSparse_s(*args)
      else:            lib.ElLAVXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElLAVSparse_d(*args)
      else:            lib.ElLAVXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElLAVDistSparse_s(*args)
      else:            lib.ElLAVXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElLAVDistSparse_d(*args)
      else:            lib.ElLAVXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# Logistic regression
# ===================
(NO_PENALTY,L1_PENALTY,L2_PENALTY)=(0,1,2)

lib.ElLogisticRegression_s.argtypes = \
lib.ElLogisticRegressionDist_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,sType,c_uint,POINTER(iType)]
lib.ElLogisticRegression_d.argtypes = \
lib.ElLogisticRegressionDist_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,dType,c_uint,POINTER(iType)]

def LogisticRegression(G,q,gamma,penalty=L1_PENALTY):
  if type(G) is not type(q):
    raise Exception('Types of G and q must match')
  if G.tag != q.tag:
    raise Exception('Datatypes of G and q must match')
  numIts = iType()
  if type(G) is Matrix:
    z = Matrix(G.tag)
    args = [G.obj,q.obj,z.obj,gamma,penalty,pointer(numIts)]
    if   G.tag == sTag: lib.ElLogisticRegression_s(*args)
    elif G.tag == dTag: lib.ElLogisticRegression_d(*args)
    else: DataExcept()
    return z, numIts
  elif type(G) is DistMatrix:
    z = DistMatrix(G.tag,MC,MR,G.Grid())
    args = [G.obj,q.obj,z.obj,gamma,penalty,pointer(numIts)]
    if   G.tag == sTag: lib.ElLogisticRegressionDist_s(*args)
    elif G.tag == dTag: lib.ElLogisticRegressionDist_d(*args)
    else: DataExcept()
    return z, numIts
  else: TypeExcept()

# Model fit
# =========
lib.ElModelFit_s.argtypes = \
lib.ElModelFitDist_s.argtypes = \
  [CFUNCTYPE(None,c_void_p,sType),CFUNCTYPE(None,c_void_p,sType),
   c_void_p,c_void_p,c_void_p,sType,POINTER(iType)]
lib.ElModelFit_d.argtypes = \
lib.ElModelFitDist_d.argtypes = \
  [CFUNCTYPE(None,c_void_p,dType),CFUNCTYPE(None,c_void_p,dType),
   c_void_p,c_void_p,c_void_p,dType,POINTER(iType)]

def ModelFit(lossProx,regProx,A,b,rho=1.2):
  if type(A) is not type(b):
    raise Exception('Types of A and b must match')
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  numIts = iType()
  cLoss = CFUNCTYPE(None,c_void_p,TagToType(A.tag))(lossProx)
  cReg = CFUNCTYPE(None,c_void_p,TagToType(A.tag))(regProx)
  if type(A) is Matrix:
    w = Matrix(A.tag)
    args = [cLoss,cReg,A.obj,b.obj,w.obj,rho,pointer(numIts)]
    if   A.tag == sTag: lib.ElModelFit_s(*args)
    elif A.tag == dTag: lib.ElModelFit_d(*args)
    else: DataExcept()
    return w, numIts
  elif type(A) is DistMatrix:
    w = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [cLoss,cReg,A.obj,b.obj,w.obj,rho,pointer(numIts)]
    if   A.tag == sTag: lib.ElModelFitDist_s(*args)
    elif A.tag == dTag: lib.ElModelFitDist_d(*args)
    else: DataExcept()
    return w, numIts
  else: TypeExcept()

# Non-negative matrix factorization
# =================================
lib.ElNMF_s.argtypes = \
lib.ElNMF_d.argtypes = \
lib.ElNMFDist_s.argtypes = \
lib.ElNMFDist_d.argtypes = \
  [c_void_p,c_void_p,c_void_p]
lib.ElNMFX_s.argtypes = \
lib.ElNMFXDist_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,QPDirectCtrl_s]
lib.ElNMFX_d.argtypes = \
lib.ElNMFXDist_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,QPDirectCtrl_d]

def NMF(A,ctrl=None):
  args = [A.obj,X.obj,Y.obj]
  argsCtrl = [A.obj,X.obj,Y.obj,ctrl]
  if type(A) is Matrix:
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNMF_s(*args)
      else:          lib.ElNMFX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNMF_d(*args)
      else:          lib.ElNMFX_d(*argsCtrl)
    else: DataExcept()
  elif type(A) is DistMatrix:
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNMFDist_s(*args)
      else:          lib.ElNMFXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNMFDist_d(*args)
      else:          lib.ElNMFXDist_d(*argsCtrl)
    else: DataExcept()
  else: TypeExcept()

# Non-negative least squares
# ==========================
lib.ElNNLS_s.argtypes = \
lib.ElNNLS_d.argtypes = \
lib.ElNNLSDist_s.argtypes = \
lib.ElNNLSDist_d.argtypes = \
lib.ElNNLSSparse_s.argtypes = \
lib.ElNNLSSparse_d.argtypes = \
lib.ElNNLSDistSparse_s.argtypes = \
lib.ElNNLSDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p]
lib.ElNNLSX_s.argtypes = \
lib.ElNNLSXDist_s.argtypes = \
lib.ElNNLSXSparse_s.argtypes = \
lib.ElNNLSXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,c_void_p,QPDirectCtrl_s]
lib.ElNNLSX_d.argtypes = \
lib.ElNNLSXDist_d.argtypes = \
lib.ElNNLSXSparse_d.argtypes = \
lib.ElNNLSXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,QPDirectCtrl_d]

def NNLS(A,b,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNNLS_s(*args)
      else:          lib.ElNNLSX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNNLS_d(*args)
      else:          lib.ElNNLSX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNNLSDist_s(*args)
      else:          lib.ElNNLSXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNNLSDist_d(*args)
      else:          lib.ElNNLSXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = SparseMatrix(A.tag)
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNNLSSparse_s(*args)
      else:          lib.ElNNLSXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNNLSSparse_d(*args)
      else:          lib.ElNNLSXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,x.obj]
    argsCtrl = [A.obj,b.obj,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl==None: lib.ElNNLSDistSparse_s(*args)
      else:          lib.ElNNLSXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl==None: lib.ElNNLSDistSparse_d(*args)
      else:          lib.ElNNLSXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# ADMM
# ----
lib.ElNNLSADMM_s.argtypes = \
lib.ElNNLSADMM_d.argtypes = \
lib.ElNNLSADMMDist_s.argtypes = \
lib.ElNNLSADMMDist_d.argtypes = \
  [c_void_p,c_void_p,c_void_p,POINTER(iType)]

def NNLSADMM(A,B):
  if type(A) is not type(B):
    raise Exception('Types of A and B must match')
  if A.tag != B.tag:
    raise Exception('Datatypes of A and B must match')
  numIts = iType()
  if type(A) is Matrix:
    X = Matrix(A.tag)
    args = [A.obj,B.obj,X.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElNNLSADMM_s(*args)
    elif A.tag == dTag: lib.ElNNLSADMM_d(*args)
    else: DataExcept()
    return X, numIts
  elif type(A) is DistMatrix:
    X = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,B.obj,X.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElNNLSADMMDist_s(*args)
    elif A.tag == dTag: lib.ElNNLSADMMDist_d(*args)
    else: DataExcept()
    return X, numIts
  else: TypeExcept()

# Basis pursuit denoising
# =======================
lib.ElBPDN_s.argtypes = \
lib.ElBPDNDist_s.argtypes = \
lib.ElBPDNSparse_s.argtypes = \
lib.ElBPDNDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p]
lib.ElBPDN_d.argtypes = \
lib.ElBPDNDist_d.argtypes = \
lib.ElBPDNSparse_d.argtypes = \
lib.ElBPDNDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p]

lib.ElBPDNX_s.argtypes = \
lib.ElBPDNXDist_s.argtypes = \
lib.ElBPDNXSparse_s.argtypes = \
lib.ElBPDNXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p,
   QPAffineCtrl_s]
lib.ElBPDNX_d.argtypes = \
lib.ElBPDNXDist_d.argtypes = \
lib.ElBPDNXSparse_d.argtypes = \
lib.ElBPDNXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p,
   QPAffineCtrl_d]

def BPDN(A,b,lambdPre,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  lambd = TagToType(A.tag)(lambdPre)
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDN_s(*args)
      else:            lib.ElBPDNX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDN_d(*args)
      else:            lib.ElBPDNX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDNDist_s(*args)
      else:            lib.ElBPDNXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDNDist_d(*args)
      else:            lib.ElBPDNXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDNSparse_s(*args)
      else:            lib.ElBPDNXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDNSparse_d(*args)
      else:            lib.ElBPDNXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,lambd,x.obj]
    argsCtrl = [A.obj,b.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElBPDNDistSparse_s(*args)
      else:            lib.ElBPDNXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElBPDNDistSparse_d(*args)
      else:            lib.ElBPDNXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# ADMM
# ----
lib.ElBPDNADMM_s.argtypes = \
lib.ElBPDNADMM_c.argtypes = \
lib.ElBPDNADMMDist_s.argtypes = \
lib.ElBPDNADMMDist_c.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p,POINTER(iType)]
lib.ElBPDNADMM_d.argtypes = \
lib.ElBPDNADMM_z.argtypes = \
lib.ElBPDNADMMDist_d.argtypes = \
lib.ElBPDNADMMDist_z.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p,POINTER(iType)]

def BPDNADMM(A,b,lamb):
  if type(A) is not type(b): raise Exception('Types of A and b must match')
  if A.tag != b.tag: raise Exception('Datatypes of A and b must match')
  numIts = iType()
  if type(A) is Matrix:
    z = Matrix(A.tag)
    args = [A.obj,b.obj,lamb,z.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElBPDNADMM_s(*args)
    elif A.tag == dTag: lib.ElBPDNADMM_d(*args)
    elif A.tag == cTag: lib.ElBPDNADMM_c(*args)
    elif A.tag == zTag: lib.ElBPDNADMM_z(*args)
    else: DataExcept()
    return z, numIts
  elif type(A) is DistMatrix:
    z = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,lamb,z.obj,pointer(numIts)]
    if   A.tag == sTag: lib.ElBPDNADMMDist_s(*args)
    elif A.tag == dTag: lib.ElBPDNADMMDist_d(*args)
    elif A.tag == cTag: lib.ElBPDNADMMDist_c(*args)
    elif A.tag == zTag: lib.ElBPDNADMMDist_z(*args)
    else: DataExcept()
    return z, numIts
  else: TypeExcept()

# Elastic net
# ===========
lib.ElEN_s.argtypes = \
lib.ElENDist_s.argtypes = \
lib.ElENSparse_s.argtypes = \
lib.ElENDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,sType,c_void_p]
lib.ElEN_d.argtypes = \
lib.ElENDist_d.argtypes = \
lib.ElENSparse_d.argtypes = \
lib.ElENDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,dType,c_void_p]

lib.ElENX_s.argtypes = \
lib.ElENXDist_s.argtypes = \
lib.ElENXSparse_s.argtypes = \
lib.ElENXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,sType,c_void_p,
   QPAffineCtrl_s]
lib.ElENX_d.argtypes = \
lib.ElENXDist_d.argtypes = \
lib.ElENXSparse_d.argtypes = \
lib.ElENXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,dType,c_void_p,
   QPAffineCtrl_d]

def EN(A,b,lambda1Pre,lambda2Pre,ctrl=None):
  if A.tag != b.tag:
    raise Exception('Datatypes of A and b must match')
  lambda1 = TagToType(A.tag)(lambda1Pre)
  lambda2 = TagToType(A.tag)(lambda2Pre)
  if type(A) is Matrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambda1,lambda2,x.obj]
    argsCtrl = [A.obj,b.obj,lambda1,lambda2,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElEN_s(*args)
      else:            lib.ElENX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElEN_d(*args)
      else:            lib.ElENX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(b) is not DistMatrix:
      raise Exception('b must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,b.obj,lambda1,lambda2,x.obj]
    argsCtrl = [A.obj,b.obj,lambda1,lambda2,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElENDist_s(*args)
      else:            lib.ElENXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElENDist_d(*args)
      else:            lib.ElENXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(b) is not Matrix:
      raise Exception('b must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,b.obj,lambda1,lambda2,x.obj]
    argsCtrl = [A.obj,b.obj,lambda1,lambda2,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElENSparse_s(*args)
      else:            lib.ElENXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElENSparse_d(*args)
      else:            lib.ElENXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(b) is not DistMultiVec:
      raise Exception('b must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,b.obj,lambda1,lambda2,x.obj]
    argsCtrl = [A.obj,b.obj,lambda1,lambda2,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElENDistSparse_s(*args)
      else:            lib.ElENXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElENDistSparse_d(*args)
      else:            lib.ElENXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# Robust Principal Component Analysis
# ===================================
lib.ElRPCA_s.argtypes = \
lib.ElRPCA_d.argtypes = \
lib.ElRPCA_c.argtypes = \
lib.ElRPCA_z.argtypes = \
lib.ElRPCADist_s.argtypes = \
lib.ElRPCADist_d.argtypes = \
lib.ElRPCADist_c.argtypes = \
lib.ElRPCADist_z.argtypes = \
  [c_void_p,c_void_p,c_void_p]

def RPCA(M):
  if type(M) is Matrix:
    L = Matrix(M.tag)
    S = Matrix(M.tag)
    args = [M.obj,L.obj,S.obj]
    if   M.tag == sTag: lib.ElRPCA_s(*args)
    elif M.tag == dTag: lib.ElRPCA_d(*args)
    elif M.tag == cTag: lib.ElRPCA_c(*args)
    elif M.tag == zTag: lib.ElRPCA_z(*args)
    return L, S
  elif type(M) is DistMatrix:
    L = DistMatrix(M.tag,MC,MR,M.Grid())
    S = DistMatrix(M.tag,MC,MR,M.Grid())
    args = [M.obj,L.obj,S.obj]
    if   M.tag == sTag: lib.ElRPCADist_s(*args)
    elif M.tag == dTag: lib.ElRPCADist_d(*args)
    elif M.tag == cTag: lib.ElRPCADist_c(*args)
    elif M.tag == zTag: lib.ElRPCADist_z(*args)
    return L, S
  else: TypeExcept()

# Sparse inverse covariance selection
# ===================================
lib.ElSparseInvCov_s.argtypes = \
lib.ElSparseInvCov_c.argtypes = \
lib.ElSparseInvCovDist_s.argtypes = \
lib.ElSparseInvCovDist_c.argtypes = \
  [c_void_p,sType,c_void_p,POINTER(iType)]
lib.ElSparseInvCov_d.argtypes = \
lib.ElSparseInvCov_z.argtypes = \
lib.ElSparseInvCovDist_d.argtypes = \
lib.ElSparseInvCovDist_z.argtypes = \
  [c_void_p,dType,c_void_p,POINTER(iType)]

def SparseInvCov(D,lamb):
  numIts = iType()
  if type(D) is Matrix:
    Z = Matrix(D.tag)
    args = [D.obj,lamb,Z.obj,pointer(numIts)]
    if   D.tag == sTag: lib.ElSparseInvCov_s(*args)
    elif D.tag == dTag: lib.ElSparseInvCov_d(*args)
    elif D.tag == cTag: lib.ElSparseInvCov_c(*args)
    elif D.tag == zTag: lib.ElSparseInvCov_z(*args)
    else: DataExcept()
    return Z, numIts
  elif type(D) is DistMatrix:
    Z = DistMatrix(D.tag,MC,MR,D.Grid())
    args = [D.obj,lamb,Z.obj,pointer(numIts)]
    if   D.tag == sTag: lib.ElSparseInvCovDist_s(*args)
    elif D.tag == dTag: lib.ElSparseInvCovDist_d(*args)
    elif D.tag == cTag: lib.ElSparseInvCovDist_c(*args)
    elif D.tag == zTag: lib.ElSparseInvCovDist_z(*args)
    else: DataExcept()
    return Z, numIts
  else: TypeExcept()

# Support Vector Machine
# ======================
lib.ElSVM_s.argtypes = \
lib.ElSVMDist_s.argtypes = \
lib.ElSVMSparse_s.argtypes = \
lib.ElSVMDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p]
lib.ElSVM_d.argtypes = \
lib.ElSVMDist_d.argtypes = \
lib.ElSVMSparse_d.argtypes = \
lib.ElSVMDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p]

lib.ElSVMX_s.argtypes = \
lib.ElSVMXDist_s.argtypes = \
lib.ElSVMXSparse_s.argtypes = \
lib.ElSVMXDistSparse_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p,
   QPAffineCtrl_s]
lib.ElSVMX_d.argtypes = \
lib.ElSVMXDist_d.argtypes = \
lib.ElSVMXSparse_d.argtypes = \
lib.ElSVMXDistSparse_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p,
   QPAffineCtrl_d]

def SVM(A,d,lambdPre,ctrl=None):
  if A.tag != d.tag:
    raise Exception('Datatypes of A and d must match')
  lambd = TagToType(A.tag)(lambdPre)
  if type(A) is Matrix:
    if type(d) is not Matrix:
      raise Exception('d must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,d.obj,lambd,x.obj]
    argsCtrl = [A.obj,d.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElSVM_s(*args)
      else:            lib.ElSVMX_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElSVM_d(*args)
      else:            lib.ElSVMX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistMatrix:
    if type(d) is not DistMatrix:
      raise Exception('d must be a DistMatrix')
    x = DistMatrix(A.tag,MC,MR,A.Grid())
    args = [A.obj,d.obj,lambd,x.obj]
    argsCtrl = [A.obj,d.obj,lambd,x.obj,ctrl] 
    if   A.tag == sTag: 
      if ctrl == None: lib.ElSVMDist_s(*args)
      else:            lib.ElSVMXDist_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElSVMDist_d(*args)
      else:            lib.ElSVMXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is SparseMatrix:
    if type(d) is not Matrix:
      raise Exception('d must be a Matrix')
    x = Matrix(A.tag)
    args = [A.obj,d.obj,lambd,x.obj]
    argsCtrl = [A.obj,d.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElSVMSparse_s(*args)
      else:            lib.ElSVMXSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElSVMSparse_d(*args)
      else:            lib.ElSVMXSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(A) is DistSparseMatrix:
    if type(d) is not DistMultiVec:
      raise Exception('d must be a DistMultiVec')
    x = DistMultiVec(A.tag,A.Comm())
    args = [A.obj,d.obj,lambd,x.obj]
    argsCtrl = [A.obj,d.obj,lambd,x.obj,ctrl]
    if   A.tag == sTag: 
      if ctrl == None: lib.ElSVMDistSparse_s(*args)
      else:            lib.ElSVMXDistSparse_s(*argsCtrl)
    elif A.tag == dTag: 
      if ctrl == None: lib.ElSVMDistSparse_d(*args)
      else:            lib.ElSVMXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

# ADMM
# ----
lib.ElSVMADMM_s.argtypes = \
lib.ElSVMADMMDist_s.argtypes = \
  [c_void_p,c_void_p,sType,c_void_p,POINTER(iType)]
lib.ElSVMADMM_d.argtypes = \
lib.ElSVMADMMDist_d.argtypes = \
  [c_void_p,c_void_p,dType,c_void_p,POINTER(iType)]

def SVMADMM(G,q,gamma):
  if type(G) is not type(q):
    raise Exception('Types of G and q must match')
  if G.tag != q.tag:
    raise Exception('Datatypes of G and q must match')
  numIts = iType()
  if type(G) is Matrix:
    z = Matrix(G.tag)
    args = [G.obj,q.obj,gamma,z.obj,pointer(numIts)]
    if   G.tag == sTag: lib.ElSVMADMM_s(*args)
    elif G.tag == dTag: lib.ElSVMADMM_d(*args)
    else: DataExcept()
    return z, numIts
  elif type(G) is DistMatrix:
    z = DistMatrix(G.tag,MC,MR,G.Grid())
    args = [G.obj,q.obj,gamma,z.obj,pointer(numIts)]
    if   G.tag == sTag: lib.ElSVMADMMDist_s(*args)
    elif G.tag == dTag: lib.ElSVMADMMDist_d(*args)
    else: DataExcept()
    return z, numIts
  else: TypeExcept()

# Total variation denoising
# =========================
lib.ElTV_s.argtypes = \
lib.ElTVDist_s.argtypes = \
lib.ElTVDistSparse_s.argtypes = \
  [c_void_p,sType,c_void_p]
lib.ElTV_d.argtypes = \
lib.ElTVDist_d.argtypes = \
lib.ElTVDistSparse_d.argtypes = \
  [c_void_p,dType,c_void_p]

lib.ElTVX_s.argtypes = \
lib.ElTVXDist_s.argtypes = \
lib.ElTVXDistSparse_s.argtypes = \
  [c_void_p,sType,c_void_p,QPAffineCtrl_s]
lib.ElTVX_d.argtypes = \
lib.ElTVXDist_d.argtypes = \
lib.ElTVXDistSparse_d.argtypes = \
  [c_void_p,dType,c_void_p,QPAffineCtrl_d]

def TV(b,lambdPre,ctrl=None):
  lambd = TagToType(b.tag)(lambdPre)
  if type(b) is Matrix:
    x = Matrix(b.tag)
    args = [b.obj,lambd,x.obj]
    argsCtrl = [b.obj,lambd,x.obj,ctrl] 
    if   b.tag == sTag: 
      if ctrl == None: lib.ElTV_s(*args)
      else:            lib.ElTVX_s(*argsCtrl)
    elif b.tag == dTag: 
      if ctrl == None: lib.ElTV_d(*args)
      else:            lib.ElTVX_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(b) is DistMatrix:
    x = DistMatrix(b.tag,MC,MR,b.Grid())
    args = [b.obj,lambd,x.obj]
    argsCtrl = [b.obj,lambd,x.obj,ctrl] 
    if   b.tag == sTag: 
      if ctrl == None: lib.ElTVDist_s(*args)
      else:            lib.ElTVXDist_s(*argsCtrl)
    elif b.tag == dTag: 
      if ctrl == None: lib.ElTVDist_d(*args)
      else:            lib.ElTVXDist_d(*argsCtrl)
    else: DataExcept()
    return x
  elif type(b) is DistMultiVec:
    x = DistMultiVec(b.tag,b.Comm())
    args = [b.obj,lambd,x.obj]
    argsCtrl = [b.obj,lambd,x.obj,ctrl]
    if   b.tag == sTag: 
      if ctrl == None: lib.ElTVDistSparse_s(*args)
      else:            lib.ElTVXDistSparse_s(*argsCtrl)
    elif b.tag == dTag: 
      if ctrl == None: lib.ElTVDistSparse_d(*args)
      else:            lib.ElTVXDistSparse_d(*argsCtrl)
    else: DataExcept()
    return x
  else: TypeExcept()

