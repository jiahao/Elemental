#
#  Copyright (c) 2009-2015, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
import El, time

n0 = n1 = 100
display = False
worldRank = El.mpi.WorldRank()

# Stack two 2D finite-difference matrices on top of each other
# and make the last column dense
#
# NOTE: Increasing the magnitudes of the off-diagonal entries by an order of
#       magnitude makes the condition number vastly higher.
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
        A.QueueLocalUpdate( sLoc, s-1, -1 )
      if x0+1 < N0:
        A.QueueLocalUpdate( sLoc, s+1, 2 )
      if x1 > 0:
        A.QueueLocalUpdate( sLoc, s-N0, -3 )
      if x1+1 < N1:
        A.QueueLocalUpdate( sLoc, s+N0, 4 )
    else:
      sRel = s-N0*N1
      x0 = sRel % N0
      x1 = sRel / N0
      A.QueueLocalUpdate( sLoc, sRel, -20 )
      if x0 > 0:
        A.QueueLocalUpdate( sLoc, sRel-1, -1.7 )
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
if display:
  El.Display( A, "A" )
  El.Display( A.DistGraph(), "Graph of A" )

y = El.DistMultiVec()
El.Uniform( y, 2*n0*n1, 1 )
if display:
  El.Display( y, "y" )
rank = El.mpi.WorldRank()
yNrm = El.Nrm2(y)
if rank == 0:
  print "|| y ||_2 =", yNrm

ctrl = El.LeastSquaresCtrl_d()
ctrl.progress = True
startLS = time.clock()
x = El.LeastSquares(A,y,ctrl)
endLS = time.clock()
if worldRank == 0:
  print "LS time:", endLS-startLS, "seconds"
xNrm = El.Nrm2(x)
if display:
  El.Display( x, "x" )
if rank == 0:
  print "|| x ||_2 =", xNrm
El.SparseMultiply(El.NORMAL,-1.,A,x,1.,y)
if display:
  El.Display( y, "A x - y" )
eNrm = El.Nrm2(y)
if rank == 0:
  print "|| A x - y ||_2 / || y ||_2 =", eNrm/yNrm

# Require the user to press a button before the figures are closed
commSize = El.mpi.Size( El.mpi.COMM_WORLD() )
El.Finalize()
if commSize == 1:
  raw_input('Press Enter to exit')
