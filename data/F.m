function res=F(x, k)
%close all;
%clc;

single res;

res = (1-x);

for i=1:k
    res = res * (1-x^(2^i));
end


