#
#  Copyright (c) 2009-2015, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
import El, time

n0 = 50
n1 = 50
display = True
worldRank = El.mpi.WorldRank()

# Stack two 2D finite-difference matrices on top of each other
# and make the last column dense
def StackedFD2D(N0,N1):
  A = El.DistSparseMatrix()
  height = 2*N0*N1
  width = N0*N1
  A.Resize(height,width)
  localHeight = A.LocalHeight()
  A.Reserve(6*localHeight)
  for sLoc in xrange(localHeight):
    s = A.GlobalRow(sLoc)
    if s < N0*N1:
      x0 = s % N0
      x1 = s / N0
      A.QueueLocalUpdate( sLoc, s, 11 )
      if x0 > 0:
        A.QueueLocalUpdate( sLoc, s-1, -10 )
      if x0+1 < N0:
        A.QueueLocalUpdate( sLoc, s+1, 20 )
      if x1 > 0:
        A.QueueLocalUpdate( sLoc, s-N0, -30 )
      if x1+1 < N1:
        A.QueueLocalUpdate( sLoc, s+N0, 40 )
    else:
      sRel = s-N0*N1
      x0 = sRel % N0
      x1 = sRel / N0
      A.QueueLocalUpdate( sLoc, sRel, -20 )
      if x0 > 0:
        A.QueueLocalUpdate( sLoc, sRel-1, -1 )
      if x0+1 < N0:
        A.QueueLocalUpdate( sLoc, sRel+1, -2 )
      if x1 > 0:
        A.QueueLocalUpdate( sLoc, sRel-N0, -3 )
      if x1+1 < N1:
        A.QueueLocalUpdate( sLoc, sRel+N0, 3 )

    # The dense last column
    A.QueueLocalUpdate( sLoc, width-1, -10/height );

  A.MakeConsistent()
  return A

A = StackedFD2D(n0,n1)
b = El.DistMultiVec()
El.Gaussian( b, 2*n0*n1, 1 )
if display:
  El.Display( A, "A" )
  El.Display( b, "b" )

ctrl = El.LPAffineCtrl_d()
ctrl.mehrotraCtrl.qsdCtrl.progress = True
ctrl.mehrotraCtrl.progress = True
ctrl.mehrotraCtrl.outerEquil = True
ctrl.mehrotraCtrl.time = True
startLAV = time.clock()
x = El.LAV( A, b, ctrl )
endLAV = time.clock()
if worldRank == 0:
  print "LAV time:", endLAV-startLAV, "seconds"
if display:
  El.Display( x, "x" )

bTwoNorm = El.Nrm2( b )
bInfNorm = El.MaxNorm( b )
r = El.DistMultiVec()
El.Copy( b, r )
El.SparseMultiply( El.NORMAL, -1., A, x, 1., r )
if display:
  El.Display( r, "r" )
rTwoNorm = El.Nrm2( r )
rOneNorm = El.EntrywiseNorm( r, 1 )
if worldRank == 0:
  print "|| b ||_2       =", bTwoNorm
  print "|| b ||_oo      =", bInfNorm
  print "|| A x - b ||_2 =", rTwoNorm
  print "|| A x - b ||_1 =", rOneNorm

startLS = time.clock()
xLS = El.LeastSquares(A,b)
endLS = time.clock()
if worldRank == 0:
  print "LS time:", endLS-startLS, "seconds"
if display:
  El.Display( xLS, "x_{LS}" )
rLS = El.DistMultiVec()
El.Copy( b, rLS )
El.SparseMultiply( El.NORMAL, -1., A, xLS, 1., rLS )
if display:
  El.Display( rLS, "A x_{LS} - b" )
rLSTwoNorm = El.Nrm2(rLS)
rLSOneNorm = El.EntrywiseNorm(rLS,1)
if worldRank == 0:
  print "|| A x_{LS} - b ||_2 =", rLSTwoNorm
  print "|| A x_{LS} - b ||_1 =", rLSOneNorm

# Require the user to press a button before the figures are closed
commSize = El.mpi.Size( El.mpi.COMM_WORLD() )
El.Finalize()
if commSize == 1:
  raw_input('Press Enter to exit')
