#!/usr/bin/env julia

using JuMP
using NLopt
using PyPlot

# -----------------------------------------------------------------------------
# m = Model(solver=NLoptSolver(algorithm=:NLOPT_LD_SLSQP))
#
# a1 = 2
# b1 = 0
# a2 = -1
# b2 = 1
#
# @defVar(m, x1)
# @defVar(m, x2 >= 0)
#
# @setNLObjective(m, Min, sqrt(x2))
# @addNLConstraint(m, x2 >= (a1*x1+b1)^3)
# @addNLConstraint(m, x2 >= (a2*x1+b2)^3)
#
# setValue(x1, 1.234)
# setValue(x2, 5.678)
#
# status = solve(m)
#
# println("got ", getObjectiveValue(m), " at ", [getValue(x1),getValue(x2)])
# -----------------------------------------------------------------------------

srand(rand(Uint))
q = [0 0; rand()*300-150 rand()*300-150; rand()*300-150 rand()*300-150; rand()*300-150 rand()*300-150]
p = [0 0; 0 300; 300 300; 300 0]

centroid_p = mean(p,1)
centroid_q = mean(q,1)

x = broadcast(-, p, centroid_p)
y = broadcast(-, q, centroid_q)

H = transpose(x)*y

U,S,V = svd(H)

id = eye(size(H,1))
id[end] = det(V*transpose(U))
r = V*id*transpose(U)

T = transpose(centroid_q) - (r*transpose(centroid_p))
sol = broadcast(+, r*transpose(p), T)
sol = transpose(sol)

differ = (sol - q).^2;
differ = transpose(differ)
differ = sum(differ,1)
differ_sq = sum(differ)
differ = sqrt(differ)
differ = sum(differ)

plot(sol[:,1],sol[:,2],color="green",linestyle="-")
plot(sol[1,1],sol[1,2],color="green",marker="o")
plot(sol[end,1],sol[end,2],color="green",marker="+")
plot(q[:,1],q[:,2],color="blue",linestyle="-")
plot(q[1,1],q[1,2],color="blue",marker="o")
plot(q[end,1],q[end,2],color="green",marker="+")

p = transpose(p)
q = transpose(q)

m = Model(solver=NLoptSolver(algorithm=:LN_BOBYQA))
m = Model()

@defVar(m, -1 <= R[1:2,1:2] <= 1)
@defVar(m, t[1:2,1:1])

@addNLConstraint(m, R[1,1]*R[2,2]-R[1,2]*R[2,1] == 1)
@addNLConstraint(m, R[1,1]*R[1,1]+R[1,2]*R[1,2] == 1)
@addNLConstraint(m, R[2,1]*R[1,1]+R[2,2]*R[1,2] == 0)
@addNLConstraint(m, R[2,1]*R[2,1]+R[2,2]*R[2,2] == 1)

#MANHATTAN DISTANCE
#@defNLExpr(obj, (sum{(sqrt((t[1,1] - q[1,i] + R[1,1]*p[1,i] + R[1,2]*p[2,i])^2) + sqrt((t[2,1] - q[2,i] + R[2,1]*p[1,i] + R[2,2]*p[2,i])^2)), i=1:size(p)[2]}))
##################################################
#SUM OF DISTANCE (L1-norm)
@defNLExpr(obj, (sum{sqrt((t[1,1] - q[1,i] + R[1,1]*p[1,i] + R[1,2]*p[2,i])^2 + (t[2,1] - q[2,i] + R[2,1]*p[1,i] + R[2,2]*p[2,i])^2), i=1:size(p)[2]}))
##################################################
#SUM OF SQUARE DISTANCE (L2-norm squared)
#@defNLExpr(obj, (sum{sqrt((t[1,1] - q[1,i] + R[1,1]*p[1,i] + R[1,2]*p[2,i])^2 + (t[2,1] - q[2,i] + R[2,1]*p[1,i] + R[2,2]*p[2,i])^2)^2, i=1:size(p)[2]}))
##################################################
#L2-norm (equivalent optimization result as above)
#@defNLExpr(obj, sqrt(sum{sqrt((t[1,1] - q[1,i] + R[1,1]*p[1,i] + R[1,2]*p[2,i])^2 + (t[2,1] - q[2,i] + R[2,1]*p[1,i] + R[2,2]*p[2,i])^2)^2, i=1:size(p)[2]}))
##################################################
@setNLObjective(m, Min, obj)

setValue(R[1,1], r[1,1])
setValue(R[1,2], r[1,2])
setValue(R[2,1], r[2,1])
setValue(R[2,2], r[2,2])
setValue(t[1,1], T[1,1])
setValue(t[2,1], T[2,1])

status = solve(m)

R11 = getValue(R[1,1])
R12 = getValue(R[1,2])
R21 = getValue(R[2,1])
R22 = getValue(R[2,2])
t11 = getValue(t[1,1])
t21 = getValue(t[2,1])
R = [R11 R12; R21 R22]
t = [t11; t21]

sol2 = broadcast(+, R*p, t)
sol2 = transpose(sol2)

differ2 = (sol2 - transpose(q)).^2;
differ2 = transpose(differ2)
differ2 = sum(differ2,1)
differ_sq2 = sum(differ2)
differ2 = sqrt(differ2)
differ2 = sum(differ2)

println("r = ", r)
println("T = ", T)
println("sum of distances (green) = ", differ)
println("sum of square distances (green) = ", differ_sq)
println("R = ", R)
println("t = ", t)
println("sum of distances (red) = ", getObjectiveValue(m))
println("sum of distances (red/check) = ", differ2)
println("sum of square distances (red) = ", differ_sq2)

plot(sol2[:,1],sol2[:,2],color="red",linestyle="--")
plot(sol2[1,1],sol2[1,2],color="red",marker="o")
plot(sol2[end,1],sol2[end,2],color="red",marker="+")

print("Hit <enter> to continue")
readline()
