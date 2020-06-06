function diff_plot_3D(fs, num, ctype, b15, bmin)
% dirs: the directory where the data file locates
% ctype = 0: 2CNF
%     = 1: Horn
%     = 2: 3CNF
% b15 = 1 % for 3CNF with 15 signatures
% b15 = 0 % for 3CNF with 10 signatures
% bmin = 1 % for the difference-MIN
close all;
clc;

z=load(char(fs)); %%% whichout 'char', it will be an error!
y = z(:,1);
% switch ctype
%     case {0}
%         %y = .1 :.1 :3; %for 2CNF
%         y = z(:,1)
%     case {1,2}
%         y =.2: .2 : 8; %for 3Horn and 3CNF
% end
if b15
    x=0:1:15; 
    if bmin
        z(:,[19]) = [];
    end
    z(:,[1,18]) = [];  % remove the first and last columns
else
    x=0:1:10;
    if bmin
        z(:,[14]) = [];
    end
    z(:,[1,13]) = [];  % remove the first and last columns
end 


%{ for multiple data
% size=1
% nums=[320 40]
% for i=1:size
%}
% remove the first clumn and the last column
% the first column is the ratio, the last column the sum

z=z'; 
h =figure;
set(h,'visible','off');

subplot(1,1,1);  
surf(y,x,z);

axis square
x1=xlabel('c');
y1=ylabel('clause length');
set(x1,'Rotation',25);
set(y1,'Rotation',-40);
ts=strcat('n=',int2str(num));
title(ts); 
 
saveas(gcf,strcat(char(fs),'.eps'));
end