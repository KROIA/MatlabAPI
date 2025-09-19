hFig = figure('Name','MySpecialFig');
drawnow;
hwnd = getHWnd(hFig);
fprintf('HWND = 0x%X\n', hwnd);
