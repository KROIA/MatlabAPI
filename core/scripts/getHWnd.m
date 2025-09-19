function hwnd = getHWnd(hFig)
%GETHWND  Return HWND of a MATLAB figure on Windows (64-bit)
%
%   hwnd = getHWnd(hFig)

    if ~ispc
        error('Windows only.');
    end
    if ~ishandle(hFig) || ~strcmp(get(hFig,'Type'),'figure')
        error('Input must be a figure handle.');
    end
    drawnow;

    titleStr = get(hFig,'Name');
    if isempty(titleStr)
        titleStr = ['Figure ' num2str(hFig.Number)];
    end

    % Load user32.dll once
    if ~libisloaded('user32')
        loadlibrary('user32.dll', @user32proto);
    end

    hwnd = calllib('user32','FindWindowA', [], titleStr);
    hwnd = uint64(hwnd);

    if hwnd == 0
        warning('getHWnd:NotFound','Could not find window with title "%s"', titleStr);
    end
end
