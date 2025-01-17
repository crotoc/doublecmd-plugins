import QtQuick 2.2

Rectangle
{
	id: root
	SystemPalette
	{
		id: pal
		colorGroup: SystemPalette.Active
	}
	anchors.centerIn: parent
	color: pal.window
	AnimatedImage
	{
		id: img
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
		onStatusChanged: playing = (status == AnimatedImage.Ready)
	}
	function myListLoad(FileToLoad, ShowFlags)
	{
		// https://ghisler.github.io/WLX-SDK/listload.htm

		/*
		console.log("myListLoad, FileToLoad:", FileToLoad, ", ShowFlags:", ShowFlags)
		console.log("Default INI: ", default_cfg)
		console.log("Plugin Directory: ", plugin_dir)

		if (ShowFlags & lcp_wraptext)
			console.log ("lcp_wraptext")
		if (ShowFlags & lcp_fittowindow)
			console.log ("lcp_fittowindow")
		if (ShowFlags & lcp_fitlargeronly)
			console.log ("lcp_fitlargeronly")
		if (ShowFlags & lcp_center)
			console.log ("lcp_center")
		if (ShowFlags & lcp_ansi)
			console.log ("lcp_ansi")
		if (ShowFlags & lcp_ascii)
			console.log ("lcp_ascii")
		if (ShowFlags & lcp_variable)
			console.log ("lcp_variable")
		if (ShowFlags & lcp_forceshow)
			console.log ("lcp_forceshow")
		*/

		img.source = FileToLoad
		if (img.status == Image.Error)
			return false
		return true
	}
	function myListLoadNext(FileToLoad, ShowFlags)
	{
		// https://ghisler.github.io/WLX-SDK/listloadnext.htm

		//console.log("myListLoadNext, FileToLoad:", FileToLoad, ", ShowFlags:", ShowFlags)

		return myListLoad(FileToLoad, ShowFlags)
	}
	function myListSendCommand(Command, Parameter)
	{
		// https://ghisler.github.io/WLX-SDK/listsendcommand.htm

		/*
		console.log("myListSendCommand Command:", Command, ", Parameter:", Parameter)
		switch (Command)
		{
		case lc_copy:
			console.log ("lc_copy")
			break
		case lc_selectall:
			console.log ("lc_selectall")
			break
		case lc_setpercent:
			console.log ("lc_setpercent")
			break
		case lc_newparams:
			if (Parameter & lcp_wraptext)
				console.log ("lcp_wraptext")
			if (Parameter & lcp_fittowindow)
				console.log ("lcp_fittowindow")
			if (Parameter & lcp_ansi)
				console.log ("lcp_ansi")
			if (Parameter & lcp_ascii)
				console.log ("lcp_ascii")
			if (Parameter & lcp_variable)
				console.log ("lcp_variable")
			if (Parameter & lcp_fitlargeronly)
				console.log ("lcp_fitlargeronly")
			break
		}
		*/

		return true
	}
	function myListSearchDialog(FindNext)
	{
		// https://ghisler.github.io/WLX-SDK/listsearchdialog.htm

		//console.log("myListSearchDialog, FindNext:", FindNext)

		return true
	}
	function myListSearchText(SearchString, SearchParameter)
	{
		// https://ghisler.github.io/WLX-SDK/listsearchtext.htm

		/*
		console.log("myListSearchDialog, SearchString:", SearchString, ", SearchParameter:", SearchParameter)

		if (SearchParameter & lcs_findfirst)
			console.log ("lcs_findfirst")
		if (SearchParameter & lcs_matchcase)
			console.log ("lcs_matchcase")
		if (SearchParameter & lcs_wholewords)
			console.log ("lcs_wholewords")
		if (SearchParameter & lcs_backwards)
			console.log ("lcs_backwards")
		*/

		return false
	}
	function myListPrint(FileToPrint, DefPrinter, PrintFlags, Margins)
	{
		// https://ghisler.github.io/WLX-SDK/listprint.htm

		//console.log("myListPrint, FileToPrint:", FileToPrint, ", DefPrinter:", DefPrinter, ", PrintFlags:", PrintFlags, ", Margins:", Margins)

		return false
	}
	function myListCloseWindow()
	{
		// https://ghisler.github.io/WLX-SDK/listclosewindow.htm

		//console.log("myListCloseWindow")
	}
}
