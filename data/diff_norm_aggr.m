function diff_norm_aggr(dir,type,ctype,b15,no_normal)
% compute the normalization of average difference 
% dir = the directory 
% type for which kind of difference
%     =1: out 
%     =2: pd 
%     =3: MIN
% ctype: the clause type
%     = 1: 2CNF
%     = 2: Horn
%     = 3: 3CNF
%     = 4 Horn 10

%%close all;
%%clc;
 


if (b15==1)
    title_str = '|A|=15';    
    %b15 = 1; % the new version for 3CNF, with 15 signatures
else
    %b15 = 0; % the old version for 3CNF, with 10 signatures
    title_str = '|A|=10';
end

switch ctype
    case {1}
        atoms = [20 40 80 160]; % 320];
        style=char(' -+r','-.g*','--xb','-.dy','-sm','-.dc'); 
        legend_str = char('n=20', 'n=40', 'n=80', 'n=160', 'n=320','n=640');
        x=.1 :.1 :3;
    case {2}
        atoms = [20 40 80 160];% 320];
        style=char(' -+r','-.g*','--xb','-.dy','-sm','-.dc'); 
        legend_str = char('n=20', 'n=40', 'n=80', 'n=160', 'n=320','n=640');
        x=.2 :.2 :8;
        
    case {3}
        if b15
          atoms = [20 40 80 160];
          style=char(' -+r','-.g*','--xb','-.dy'); 
          legend_str = char('n=20', 'n=40', 'n=80', 'n=160');
        else
            atoms = [20 40 80 160]; %[10 20 40 80 160];
            style=char(' -+r','-.g*','--xb','-.dy','-sm'); 
            legend_str = char('n=20', 'n=40', 'n=80', 'n=160');
            title_str = '|A|=10';
        end
        x=.2 :.2 :8;
    case {4} % for Horn with 10 relative signature
        atoms = [10 15];
        style=char(' -+r','-.g*'); 
        legend_str = char('n=10', 'n=15');
end

[r,len] = size(atoms); % the number of curcuives for aggregation
types=char('cd', 'pd', 'MIN');
yls = char('clausal', 'prime', 'minimal');
ylabel_str = strcat(yls(type,:), ' difference');

if no_normal == 1
    s_fn =strcat(dir,'\', 'norm-aggr-difference-',types(type,:));
else
    s_fn =strcat(dir,'\', 'aggr-difference-',types(type,:));
end

h =figure;
set(h,'visible','off');
subplot(1,1,1);
for j=1:len
    % get the file name
    fn=strcat(dir,'\',int2str(atoms(j)),'\', 'difference-',types(type,:));

    if ~exist(fn,'file')
         fn=strcat(dir,'\',int2str(atoms(j)),'\', 'difference-out');
    end
    A=load(fn);
    [row, col] = size(A);   
    x = A(:,1); % the first column of the data
    if type == 3 % for MIN
        y = A(:,col-1)';
    else
        y = A(:,col)';
    end
    % normalization the aggregated difference
    if (no_normal == 1 & max(y) > min(y)) 
        y = (y -min(y)) / (max(y)-min(y));
    end
    % for matlab version > 7.0    
    %{
    [ny,ps]=mapminmax(y);
    ps.ymin=0;
    [nny,ps]=mapminmax(y,ps);
    %}
    plot(x, y, style(j,:)); 
    hold all
end
axis square
y1=ylabel(ylabel_str);
x1=xlabel('c');
title(title_str);
%text(4.6,.15, '\leftarrow critical point')
if ctype ~= 2
    gca = legend(legend_str);% [po(1)-po(4)+20, po(2)-po(3)+10, po(3), po(4)]); %'n=10','n=20','n=40','n=80','n=160')
else
    gca =legend(legend_str,'location','EastOutside');
end

po=get( gca, 'Position' ); % get the position of legend
if (ctype ~= 2)
    set( gca, 'Position', [po(1)-0.08, po(2), po(3), po(4)] ); % set the position of legend
%else
%     set( gca, 'Position', [po(1)-0.001, po(2), po(3), po(4)] ); % set the position of legend for Horn
% elseif (type ~= 1) 
%     set( gca, 'Position', [po(1)-0.5, po(2), po(3), po(4)] ); % set the position of legend for Horn
% else
%     set( gca, 'Position', [po(1)-0.08, po(2)+0.05, po(3), po(4)] ); % set the position of legend for Horn
end
grid on

saveas(gcf,strcat(s_fn,'.eps'));
end