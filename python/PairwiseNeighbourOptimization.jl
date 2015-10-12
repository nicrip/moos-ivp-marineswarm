#!/usr/bin/env julia

using JuMP
using NLopt
using PyPlot

type Point{T}
  x::T
  y::T
end

function relAng(xa, ya, xb, yb)
    if((xa==xb)&&(ya==yb))
    	ang = 0
      return ang
    end

    w   = 0
    sop = 0

    if(xa < xb)
        if(ya==yb)
          ang = 90.0
          return ang
        else
          w = 90.0
        end
    end

    if(xa > xb)
        if(ya==yb)
          ang = 270.0
          return ang
        else
          w = 270.0
        end
    end

    if(ya < yb)
        if(xa == xb)
          ang = 0.0
          return ang
        end
        if(xb > xa)
          sop = -1.0
        else
          sop =  1.0
        end
    end

    if(yb < ya)
        if(xa == xb)
          ang = 180
          return ang
        end
        if(xb >  xa)
          sop =  1.0
        else
          sop = -1.0
        end
    end

    ydiff = yb-ya
    xdiff = xb-xa
    if(ydiff<0)
        ydiff = ydiff * -1.0
    end

    if(xdiff<0)
        xdiff = xdiff * -1.0
    end

    avalPI = atan(ydiff/xdiff)
    avalDG = (avalPI)*180/pi
    retVal = (avalDG * sop) + w

    while(retVal >= 360.0)
        retVal = retVal - 360.0
    end
    while(retVal < 0.0)
        retVal = retVal + 360.0
    end

   ang = retVal
   return ang
end

srand(rand(Uint))
nb1 = Point{Float64}(rand()*300-150, rand()*300-150)
nb2 = Point{Float64}(rand()*300-150, rand()*300-150)
nb3 = Point{Float64}(rand()*300-150, rand()*300-150)
nb1 = Point{Float64}(310, 250)
nb2 = Point{Float64}(220, 20)
nb3 = Point{Float64}(10, 50)
#nb1 = Point{Float64}(0, 300)
#nb2 = Point{Float64}(300, 300)
#nb3 = Point{Float64}(300, 0)

nb1_plan = Point{Float64}(0, 300)
nb2_plan = Point{Float64}(300, 300)
nb3_plan = Point{Float64}(300, 0)

th_o_nb1 = atan2(nb1.y,nb1.x)
th_o_nb2 = atan2(nb2.y,nb2.x)
th_o_nb3 = atan2(nb3.y,nb3.x)
r_o_nb1 = hypot(nb1.y,nb1.x)
r_o_nb2 = hypot(nb2.y,nb2.x)
r_o_nb3 = hypot(nb3.y,nb3.x)

mid_n1_n2 = Point{Float64}((nb1.x + nb2.x)/2,(nb1.y + nb2.y)/2);
mid_n2_n3 = Point{Float64}((nb2.x + nb3.x)/2,(nb2.y + nb3.y)/2);
mid_n1_n3 = Point{Float64}((nb1.x + nb3.x)/2,(nb1.y + nb3.y)/2);

mid_n1_n2_plan = Point{Float64}((nb1_plan.x + nb2_plan.x)/2,(nb1_plan.y + nb2_plan.y)/2);
mid_n2_n3_plan = Point{Float64}((nb2_plan.x + nb3_plan.x)/2,(nb2_plan.y + nb3_plan.y)/2);
mid_n1_n3_plan = Point{Float64}((nb1_plan.x + nb3_plan.x)/2,(nb1_plan.y + nb3_plan.y)/2);

dist_mid_n1_n2_plan = hypot(mid_n1_n2_plan.x, mid_n1_n2_plan.y);
dist_mid_n2_n3_plan = hypot(mid_n2_n3_plan.x, mid_n2_n3_plan.y);
dist_mid_n1_n3_plan = hypot(mid_n1_n3_plan.x, mid_n1_n3_plan.y);

angle_mid_n1_n2_plan = relAng(mid_n1_n2_plan.x, mid_n1_n2_plan.y, 0, 0) - relAng(nb1_plan.x, nb1_plan.y, nb2_plan.x, nb2_plan.y);
angle_mid_n2_n3_plan = relAng(mid_n2_n3_plan.x, mid_n2_n3_plan.y, 0, 0) - relAng(nb2_plan.x, nb2_plan.y, nb3_plan.x, nb3_plan.y);
angle_mid_n1_n3_plan = relAng(mid_n1_n3_plan.x, mid_n1_n3_plan.y, 0, 0) - relAng(nb1_plan.x, nb1_plan.y, nb3_plan.x, nb3_plan.y);

angle_project_n1_n2 = angle_mid_n1_n2_plan + relAng(nb1.x, nb1.y, nb2.x, nb2.y);
angle_project_n2_n3 = angle_mid_n2_n3_plan + relAng(nb2.x, nb2.y, nb3.x, nb3.y);
angle_project_n1_n3 = angle_mid_n1_n3_plan + relAng(nb1.x, nb1.y, nb3.x, nb3.y);

x_n1_n2 = mid_n1_n2.x + dist_mid_n1_n2_plan*sin(angle_project_n1_n2*pi/180);
y_n1_n2 = mid_n1_n2.y + dist_mid_n1_n2_plan*cos(angle_project_n1_n2*pi/180);

x_n2_n3 = mid_n2_n3.x + dist_mid_n2_n3_plan*sin(angle_project_n2_n3*pi/180);
y_n2_n3 = mid_n2_n3.y + dist_mid_n2_n3_plan*cos(angle_project_n2_n3*pi/180);

x_n1_n3 = mid_n1_n3.x + dist_mid_n1_n3_plan*sin(angle_project_n1_n3*pi/180);
y_n1_n3 = mid_n1_n3.y + dist_mid_n1_n3_plan*cos(angle_project_n1_n3*pi/180);

println("centroid: ")
println("x = ", (x_n1_n2 + x_n2_n3 + x_n1_n3)/3, " y = ", (y_n1_n2 + y_n2_n3 + y_n1_n3)/3)

plot([nb1.x; nb2.x; nb3.x],[nb1.y; nb2.y; nb3.y], color="blue",linestyle="-");
plot(nb1.x,nb1.y, color="blue",marker="o");
plot([mid_n1_n2.x; mid_n2_n3.x; mid_n1_n3.x],[mid_n1_n2.y; mid_n2_n3.y; mid_n1_n3.y], "red",marker="x",linestyle="");
plot([x_n1_n2; x_n2_n3; x_n1_n3; x_n1_n2], [y_n1_n2; y_n2_n3; y_n1_n3; y_n1_n2], "green",marker="o",linestyle="-");
plot((x_n1_n2 + x_n2_n3 + x_n1_n3)/3,(y_n1_n2 + y_n2_n3 + y_n1_n3)/3, color="black",marker="o");

m = Model()

@defVar(m, n1_n2[1:1,1:2])
@defVar(m, n2_n3[1:1,1:2])
@defVar(m, n1_n3[1:1,1:2])

@addConstraint(m,  n1_n2[1,1] + mid_n1_n2.x == n1_n3[1,1] + mid_n1_n3.x)
@addConstraint(m,  n1_n2[1,2] + mid_n1_n2.y == n1_n3[1,2] + mid_n1_n3.y)
@addConstraint(m,  n1_n2[1,1] + mid_n1_n2.x == n2_n3[1,1] + mid_n2_n3.x)
@addConstraint(m,  n1_n2[1,2] + mid_n1_n2.y == n2_n3[1,2] + mid_n2_n3.y)
@addConstraint(m,  n1_n3[1,1] + mid_n1_n3.x == n2_n3[1,1] + mid_n2_n3.x)
@addConstraint(m,  n1_n3[1,2] + mid_n1_n3.y == n2_n3[1,2] + mid_n2_n3.y)

@defNLExpr(  obj, ((((n1_n2[1,1]^2) + (n1_n2[1,2]^2)) - dist_mid_n1_n2_plan^2)^2) + ((((n2_n3[1,1]^2) + (n2_n3[1,2]^2)) - dist_mid_n2_n3_plan^2)^2) + ((((n1_n3[1,1]^2) + (n1_n3[1,2]^2)) - dist_mid_n1_n3_plan^2)^2) )
@setNLObjective(m, Min, obj)

setValue(n1_n2[1,1], dist_mid_n1_n2_plan*sin(angle_project_n1_n2*pi/180))
setValue(n1_n2[1,2], dist_mid_n1_n2_plan*cos(angle_project_n1_n2*pi/180))
setValue(n1_n3[1,1], dist_mid_n2_n3_plan*sin(angle_project_n2_n3*pi/180))
setValue(n1_n3[1,2], dist_mid_n2_n3_plan*cos(angle_project_n2_n3*pi/180))
setValue(n2_n3[1,1], dist_mid_n1_n3_plan*sin(angle_project_n1_n3*pi/180))
setValue(n2_n3[1,2], dist_mid_n1_n3_plan*cos(angle_project_n1_n3*pi/180))

#setValue(n1_n2[1,1], 1000)
#setValue(n1_n2[1,2], -1000)
#setValue(n1_n3[1,1], 1000)
#setValue(n1_n3[1,2], -1000)
#setValue(n2_n3[1,1], 1000)
#setValue(n2_n3[1,2], -1000)

status = solve(m)

println("sum of magnitudes = ", getObjectiveValue(m))

n1_n2_11 = getValue(n1_n2[1,1])
n1_n2_12 = getValue(n1_n2[1,2])
x_n1_n2 = mid_n1_n2.x + n1_n2_11;
y_n1_n2 = mid_n1_n2.y + n1_n2_12;

n2_n3_11 = getValue(n2_n3[1,1])
n2_n3_12 = getValue(n2_n3[1,2])
x_n2_n3 = mid_n2_n3.x + n2_n3_11;
y_n2_n3 = mid_n2_n3.y + n2_n3_12;

n1_n3_11 = getValue(n1_n3[1,1])
n1_n3_12 = getValue(n1_n3[1,2])
x_n1_n3 = mid_n1_n3.x + n1_n3_11;
y_n1_n3 = mid_n1_n3.y + n1_n3_12;

println(n1_n2_11," ",n1_n2_12," ",n2_n3_11," ",n2_n3_12," ",n1_n3_11," ",n1_n3_12)

println("mag n1_n2 = ", sqrt(n1_n2_11^2 + n1_n2_12^2), " vs. ", dist_mid_n1_n2_plan)
println("mag n1_n3 = ", sqrt(n1_n3_11^2 + n1_n3_12^2), " vs. ", dist_mid_n1_n3_plan)
println("mag n2_n3 = ", sqrt(n2_n3_11^2 + n2_n3_12^2), " vs. ", dist_mid_n2_n3_plan)
println((sqrt(n1_n2_11^2 + n1_n2_12^2) - dist_mid_n1_n2_plan )^2)

println("optimization: ")
println("x = ", x_n1_n2, " y = ", y_n1_n2)
println("x = ", x_n1_n3, " y = ", y_n1_n3)
println("x = ", x_n2_n3, " y = ", y_n2_n3)
plot(x_n1_n2,y_n1_n2, color="black",marker="x");

print("Hit <enter> to continue")
readline()
