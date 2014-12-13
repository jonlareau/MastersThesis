function GMMDemo
%Demo showcasing Gaussian Mixture Models.
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

%Make Sample Data
rad = 0:.009:2*pi;
mag = rand(size(rad))+5;

y = mag.*sin(rad);
x = mag.*cos(rad);
feat = [x;y];

subplot(2,2,1),plot(x,y,'b. '); axis square; axis([-7,7,-7,7]);
title('Raw Data Points');
drawnow;

%Now make GMM approximations to the data distribution using various numbers
%of mixtures...

ncentres = 4;
subplot(2,2,2),
[mix, options, errlog] = JL_MAKE_GMM(feat,ncentres);
% Plot the result
x = -7.0:0.2:7.0;
y = -7.0:0.2:7.0;
[X, Y] = meshgrid(x,y);
X = X(:);
Y = Y(:);
grid = [X Y];
Z = gmmprob(mix, grid);
Z = reshape(Z, length(x), length(y));
c = mesh(x, y, Z);
view(2);
axis square; axis tight;
hold on
title(['GMM N:',num2str(ncentres)]);
hold off
drawnow;

ncentres = 16;
subplot(2,2,3),
[mix, options, errlog] = JL_MAKE_GMM(feat,ncentres);
% Plot the result
x = -7.0:0.2:7.0;
y = -7.0:0.2:7.0;
[X, Y] = meshgrid(x,y);
X = X(:);
Y = Y(:);
grid = [X Y];
Z = gmmprob(mix, grid);
Z = reshape(Z, length(x), length(y));
c = mesh(x, y, Z);
view(2);
axis square; axis tight;
hold on
title(['GMM N:',num2str(ncentres)]);
hold off
drawnow;

ncentres = 32;
subplot(2,2,4),
[mix, options, errlog] = JL_MAKE_GMM(feat,ncentres);
% Plot the result
x = -7.0:0.2:7.0;
y = -7.0:0.2:7.0;
[X, Y] = meshgrid(x,y);
X = X(:);
Y = Y(:);
grid = [X Y];
Z = gmmprob(mix, grid);
Z = reshape(Z, length(x), length(y));
c = mesh(x, y, Z);
view(2);
axis square; axis tight;
hold on
title(['GMM N:',num2str(ncentres)]);
hold off
drawnow;
