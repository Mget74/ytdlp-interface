#include "../gui.hpp"


void GUI::fm_formats()
{
	using namespace nana;
	using ::widgets::theme;
	auto url {bottoms.visible()};
	auto &bottom {bottoms.at(url)};
	auto &vidinfo {bottom.vidinfo};

	themed_form fm {nullptr, *this, {}, appear::decorate<appear::minimize, appear::sizable>{}};
	fm.caption(title + " - manual selection of formats");
	fm.bgcolor(theme::fmbg);
	fm.snap(conf.cbsnap);
	fm.div(R"(
			vert margin=20
				<weight=180px 
					<switchable weight=320px <thumb> <thumb_label>> <weight=20px>
					<vert
						<l_title weight=27px> <weight=10px>
						<sep1 weight=3px> <weight=10px>
						<weight=28px <l_dur weight=45%> <weight=10> <l_durtext> >
						<weight=28px <l_chap weight=45%> <weight=10> <l_chaptext> >
						<weight=28px <l_upl weight=45%> <weight=10> <l_upltext> >
						<weight=28px <l_date weight=45%> <weight=10> <l_datetext> > <weight=15px>
						<sep2 weight=3px>
					>
				>
				<weight=20> <cb_streams weight=25>
				<weight=20> <list> <weight=20>
				<weight=35 <><btncancel weight=420> <weight=20> <btnok weight=420><>>
		)");

	::widgets::Title l_title {fm};
	::widgets::Label l_dur {fm, "Duration:", true}, l_chap {fm, "Chapters:", true},
		l_upl {fm, "Uploader:", true}, l_date {fm, "Upload date:", true};
	::widgets::Text l_durtext {fm, "", true}, l_chaptext {fm, "", true}, l_upltext {fm, "", true}, l_datetext {fm, "", true};
	nana::picture thumb {fm};
	::widgets::thumb_label thumb_label {fm};
	::widgets::Separator sep1 {fm}, sep2 {fm};
	::widgets::cbox cb_streams {fm, "Select multiple audio formats to merge into .mkv file as audio tracks "
		"(this passes --audio-multistreams to yt-dlp)"};
	::widgets::Listbox list {fm, true};
	::widgets::Button btnok {fm, "Use the selected format(s)"}, btncancel {fm, "Let yt-dlp choose the best formats (default)"};

	fm["l_title"] << l_title;
	fm["l_dur"] << l_dur;
	fm["l_durtext"] << l_durtext;
	fm["l_chap"] << l_chap;
	fm["l_chaptext"] << l_chaptext;
	fm["l_upl"] << l_upl;
	fm["l_upltext"] << l_upltext;
	fm["l_date"] << l_date;
	fm["l_datetext"] << l_datetext;
	fm["thumb"] << thumb;
	fm["thumb_label"] << thumb_label;
	fm["sep1"] << sep1;
	fm["sep2"] << sep2;
	fm["cb_streams"] << cb_streams;
	fm["list"] << list;
	fm["btncancel"] << btncancel;
	fm["btnok"] << btnok;

	cb_streams.check(conf.audio_multistreams);

	list.sortable(false);
	list.checkable(true);
	list.hilight_checked(true);
	list.enable_single(true, false);
	list.scheme().text_margin = util::scale(10) + (nana::api::screen_dpi(true) > 96) * 4;
	list.append_header("format", dpi_transform(280));
	list.append_header("acodec", dpi_transform(90));
	list.append_header("vcodec", dpi_transform(90));
	list.append_header("ext", dpi_transform(50));
	list.append_header("fps", dpi_transform(32));
	list.append_header("vbr", dpi_transform(40));
	list.append_header("abr", dpi_transform(40));
	list.append_header("asr", dpi_transform(50));
	list.append_header("filesize", dpi_transform(cnlang ? 170 : 160));

	list.events().selected([&](const arg_listbox &arg)
	{
		if(arg.item.selected())
			arg.item.check(!arg.item.checked()).select(false);
	});

	list.events().mouse_down([&](const arg_mouse &arg)
	{
		if(conf.audio_multistreams)
		{
			auto pos {list.cast({arg.pos.x, arg.pos.y})};
			if(pos.is_category() && list.at(pos.cat).text() == "Audio only")
			{
				list.auto_draw(false);
				for(auto ip : list.at(pos.cat))
					ip.check(!ip.checked());
				list.auto_draw(true);
			}
		}
	});

	list.events().checked([&](const arg_listbox &arg)
	{
		auto item {arg.item};
		auto pos {item.pos()};

		if(item.checked())
		{
			item.fgcolor(theme::list_check_highlight_fg);
			item.bgcolor(theme::list_check_highlight_bg);
			list.auto_draw(false);
			if(pos.cat == 0)
			{
				for(auto ip : list.at(0))
					if(ip != item)
						ip.check(false);
				if(list.size_categ() == 3)
					for(auto ip : list.at(2))
						ip.check(false);
				if(list.size_categ() > 1 && (!conf.audio_multistreams || list.at(1).text() == "Video only"))
					for(auto ip : list.at(1))
						ip.check(false);
			}
			else if(list.size_categ() > 1)
			{
				if(list.at(pos.cat).text() == "Video only")
				{
					for(auto ip : list.at(0))
						ip.check(false);
					for(auto ip : list.at(2))
						if(ip != item)
							ip.check(false);
				}
				else if(!conf.audio_multistreams)
				{
					for(auto ip : list.at(1))
						if(ip != item)
							ip.check(false);
					for(auto ip : list.at(0))
						if(ip != item)
							ip.check(false);
				}
			}
			list.auto_draw(true);
		}
		else
		{
			item.fgcolor(list.fgcolor());
			item.bgcolor(list.bgcolor());
		}
		btnok.enable(!list.checked().empty());
	});

	cb_streams.events().checked([&]
	{
		conf.audio_multistreams = cb_streams.checked();
		if(!conf.audio_multistreams && list.size_categ() > 1 && list.at(1).text() == "Audio only")
		{
			list.auto_draw(false);
			int count {0};
			for(auto ip : list.at(0))
				if(ip.checked())
				{
					count++;
					break;
				}
			for(auto ip : list.at(1))
				if(ip.checked())
				{
					if(!count)
					{
						count++;
						continue;
					}
					ip.check(false);
				}
			list.auto_draw(true);
		}
	});

	btncancel.events().click([&, this]
	{
		auto &bottom {bottoms.current()};
		bottom.use_strfmt = false;
		fm.close();
	});

	auto get_int = [](const nlohmann::json &j, const std::string &key) -> std::string
	{
		return (j.contains(key) && j[key] != nullptr) ? std::to_string(j[key].get<unsigned>()) : "---";
	};

	auto get_string = [](const nlohmann::json &j, const std::string &key) -> std::string
	{
		return (j.contains(key) && j[key] != nullptr) ? j[key].get<std::string>() : "---";
	};

	btnok.events().click([&, this]
	{
		auto &bottom {bottoms.current()};
		auto &strfmt {bottom.strfmt}, &fmt1 {bottom.fmt1}, &fmt2 {bottom.fmt2};
		fmt1.clear();
		fmt2.clear();

		size_t vidcat {0};
		if(list.size_categ() == 3)
			vidcat = 2;
		else if(list.size_categ() == 2 && list.at(1).text() == "Video only")
			vidcat = 1;

		bool mergeall {false};
		if(vidcat == 2)
		{
			size_t cnt {0};
			for(const auto ip : list.at(1))
				if(ip.checked())
					cnt++;
			if(list.at(1).size() == cnt)
			{
				fmt2 = L"mergeall";
				mergeall = true;
			}
		}

		auto sel {list.checked()};
		for(const auto &pos : sel)
		{
			auto &val {list.at(pos).value<std::wstring>()};
			if(pos.cat == vidcat || pos.cat == 0)
			{
				fmt1 = std::move(val);
				if(mergeall) break;
			}
			else if(!mergeall)
			{
				if(!fmt2.empty())
					fmt2 += '+';
				fmt2 += std::move(val);
			}
		}
		strfmt = fmt1;
		if(!fmt2.empty())
		{
			if(!strfmt.empty())
				strfmt += L'+';
			strfmt += fmt2;
		}

		conf.fmt1 = fmt1;
		conf.fmt2 = fmt2;

		if(sel.size() == 1)
		{
			auto item {lbq.at(lbq.selected().front())};
			auto it {std::find_if(vidinfo["formats"].begin(), vidinfo["formats"].end(), [&](const auto &el)
			{
				return el["format"].get<std::string>().rfind(nana::to_utf8(strfmt)) != -1;
			})};
			std::string fsize {"---"}, fmt_note, ext, fmtid;
			if(it != vidinfo["formats"].end())
			{
				auto &fmt {*it};
				if(fmt.contains("filesize") && fmt["filesize"] != nullptr)
					fsize = util::int_to_filesize(fmt["filesize"], false);
				if(list.at(sel.front().cat).text() == "Audio only")
					fmt_note = get_string(fmt, "format_note");
				else fmt_note = get_string(fmt, "resolution");
				ext = get_string(fmt, "ext");
				fmtid = get_string(fmt, "format_id");
				item.text(4, fmtid);
				item.text(5, fmt_note);
				item.text(6, ext);
				if(fsize != "---")
					fsize = '~' + fsize;
				item.text(7, fsize);
			}
		}

		bottom.use_strfmt = true;
		if(bottom.using_custom_fmt())
		{
			nana::msgbox mbox {fm, "Warning: conflicting -f arguments"};
			std::string text {"The \"Custom arguments\" checkbox is checked, and \"-f\" is present as a custom argument.\n\n"
				"If you don't uncheck that checkbox, the \"-f\" custom argument will override the selection you have made here."};
			mbox.icon(nana::msgbox::icon_warning);
			(mbox << text)();
		}
		fm.close();
	});

	btnok.enabled(false);
	thumb.transparent(true);

	std::string thumb_url, title {"[title missing]"};
	if(bottom.vidinfo_contains("title"))
		title = vidinfo["title"];
	if(!bottom.is_ytlink)
	{
		if(bottom.vidinfo_contains("thumbnail"))
			thumb_url = vidinfo["thumbnail"];
		thumb.stretchable(true);
		thumb.align(align::center, align_v::center);
	}
	else
	{
		auto it {std::find_if(vidinfo["thumbnails"].begin(), vidinfo["thumbnails"].end(), [](const auto &el)
		{
			return el["url"].get<std::string>().rfind("mqdefault.jpg") != -1;
		})};

		if(it == vidinfo["thumbnails"].end())
			it = std::find_if(vidinfo["thumbnails"].begin(), vidinfo["thumbnails"].end(), [](const auto &el)
		{
			return el["url"].get<std::string>().rfind("mqdefault_live.jpg") != -1;
		});

		if(it != vidinfo["thumbnails"].end())
			thumb_url = (*it)["url"];
	}

	l_title.caption(title);
	list.append({"Audio only", "Video only"});

	int dur {0};
	bool live {bottom.vidinfo_contains("is_live") && bottom.vidinfo["is_live"] ||
			bottom.vidinfo_contains("live_status") && bottom.vidinfo["live_status"] == "is_live"};
	if(!live && bottom.vidinfo_contains("duration"))
		dur = vidinfo["duration"];
	int hr {(dur / 60) / 60}, min {(dur / 60) % 60}, sec {dur % 60};
	if(dur < 60) sec = dur;
	std::string durstr {live ? "live" : "---"};
	if(dur)
	{
		std::stringstream ss;
		ss.width(2);
		ss.fill('0');
		if(hr)
		{
			ss << min;
			durstr = std::to_string(hr) + ':' + ss.str();
			ss.str("");
			ss.width(2);
			ss.fill('0');
		}
		else durstr = std::to_string(min);
		ss << sec;
		durstr += ':' + ss.str();
	}
	else if(bottom.vidinfo_contains("duration_string"))
		durstr = vidinfo["duration_string"];
	l_durtext.caption(durstr);

	std::string strchap;
	if(bottom.vidinfo_contains("chapters"))
		strchap = std::to_string(vidinfo["chapters"].size());
	if(strchap.empty() || strchap == "0")
		l_chaptext.caption("none");
	else l_chaptext.caption(strchap);

	if(bottom.vidinfo_contains("uploader"))
		l_upltext.caption(std::string {vidinfo["uploader"]});
	else l_upltext.caption("---");

	std::string strdate {"---"};
	if(bottom.vidinfo_contains("upload_date"))
	{
		strdate = vidinfo["upload_date"];
		strdate = strdate.substr(0, 4) + '-' + strdate.substr(4, 2) + '-' + strdate.substr(6, 2);
	}
	l_datetext.caption(strdate);

	if(!thumb_url.empty())
	{
		thr_thumb = std::thread([&]
		{
			thumbthr_working = true;
			std::string error;
			auto thumb_jpg {util::get_inet_res(thumb_url, &error)};
			if(!thumb_jpg.empty())
			{
				paint::image img;
				img.open(thumb_jpg.data(), thumb_jpg.size());
				if(img.empty())
				{
					std::string ext;
					auto pos {thumb_url.rfind('.')};
					if(pos != -1)
					{
						auto idx {pos};
						while(++idx < thumb_url.size() && isalpha(thumb_url[idx]));
						ext = thumb_url.substr(pos, idx-pos);
					}
					if(ext.empty())
						thumb_label.caption("thumbnail format unsupported");
					else thumb_label.caption("thumbnail format unsupported\n(" + ext + ')');
					fm.get_place().field_display("thumb_label", true);
					fm.collocate();
				}
				else if(thumbthr_working)
					thumb.load(img);
				thumbthr_working = false;
				if(thr_thumb.joinable())
					thr_thumb.detach();
			}
			else
			{
				if(error.empty())
					thumb_label.caption("error downloading thumbnail");
				else thumb_label.caption(error);
				fm.get_place().field_display("thumb_label", true);
				fm.collocate();
			}
		});
	}
	else
	{
		thumb_label.caption("thumbnail not available");
		fm.get_place().field_display("thumb_label", true);
		fm.collocate();
	}

	std::string format_id1, format_id2;
	if(bottom.vidinfo_contains("format_id"))
	{
		std::string format_id {vidinfo["format_id"]};
		auto pos(format_id.find('+'));
		if(pos != -1)
		{
			format_id1 = format_id.substr(0, pos);
			format_id2 = format_id.substr(pos + 1);
		}
		else format_id1 = format_id;
	}

	std::vector<bool> colmask(9, false);
	for(auto &fmt : vidinfo["formats"])
	{
		std::string format {fmt["format"]}, acodec, vcodec, ext, fps, vbr, abr, asr, filesize {"---"};
		if(format.find("storyboard") == -1)
		{
			abr = get_int(fmt, "abr");
			vbr = get_int(fmt, "vbr");
			asr = get_int(fmt, "asr");
			fps = get_int(fmt, "fps");
			ext = get_string(fmt, "ext");
			acodec = get_string(fmt, "acodec");
			vcodec = get_string(fmt, "vcodec");
			if(fmt.contains("filesize") && fmt["filesize"] != nullptr)
				filesize = util::int_to_filesize(fmt["filesize"]);
			unsigned catidx {0};
			if(acodec == "none")
				catidx = 2; // video only
			else if(vcodec == "none")
				catidx = 1; // audio only
			list.at(catidx).append({format, acodec, vcodec, ext, fps, vbr, abr, asr, filesize});
			auto idstr {to_wstring(fmt["format_id"].get<std::string>())};
			auto item {list.at(catidx).back()};
			item.value(idstr);
			if(idstr == conf.fmt1 || (conf.audio_multistreams ? conf.fmt2.find(idstr) != -1 : idstr == conf.fmt2))
				list.at(catidx).back().select(true);
			if(!format_id1.empty() && format_id1 == to_utf8(idstr))
				item.text(0, format + " *");
			if(!format_id2.empty() && format_id2 == to_utf8(idstr))
				item.text(0, format + " *");
			for(int n {1}; n < 9; n++)
			{
				const auto text {item.text(n)};
				if(!colmask[n] && text != "---" && text != "none")
					colmask[n] = true;
			}
		}
	}

	for(int n {1}; n < 9; n++)
		list.column_at(n).visible(colmask[n]);

	if(list.size_categ() == 3)
	{
		if(list.at(2).size() == 0)
			list.erase(2);
		if(list.at(1).size() == 0)
			list.erase(1);
	}

	list.refresh_theme();

	fm.events().unload([&]
	{
		if(thr_thumb.joinable())
		{
			thumbthr_working = false;
			thr_thumb.detach();
		}
	});

	fm.theme_callback([&, this](bool dark)
	{
		apply_theme(dark);
		fm.bgcolor(theme::fmbg);
		return false;
	});

	if(conf.cbtheme == 2)
		fm.system_theme(true);
	else fm.dark_theme(conf.cbtheme == 0);

	fm.center(1000, std::max(429.0 + list.item_count() * (cnlang ? 22.5 : 20.5), double(600)));
	api::track_window_size(fm, dpi_transform_size(900, 600), false);

	fm.collocate();
	fm.modality();
}