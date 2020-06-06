function plot_3Ds(dirs,type,b15)
%type = 0: 2CNF
%     = 1: Horn
%     = 2: 3CNF
% b15 = 1 % the new version for 3CNF, with 15 signatures
% b15 = 0 %the old version for 3CNF, with 10 signatures 

%atoms = [20 40 80 160 ];
atoms = [15];
% switch type
%     case {0}
%         atoms = [20 40 80 160 ];% 320 640]; 
%     case {1}
%         atoms = [20 40 80 160]; % 320]; 
%     case {2}
%         if b15==1
%           atoms = [20 40 80 160]; 
%         else
%             atoms = [10 20 40 80 160]; 
%        end
%end
[z,len] = size(atoms);
  types = {'cd', 'pd', 'MIN'};
  for i=1:len
      for j=1:3
          fn = strcat(dirs,'\',int2str(atoms(i)),'\difference-',types(j));  
          if strcmp(types(j),'MIN')
              bmin = 1;
          else
              bmin = 0;
          end
          diff_plot_3D(fn, atoms(i), type, b15, bmin); 
      end
  end
end