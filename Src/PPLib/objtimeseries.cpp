// OBJTIMESERIES.CPP
// Copyright (c) A.Sobolev 2018
// @codepage UTF-8
// Модуль, управляющий объектом данных PPObjTimeSeries
//
#include <pp.h>
#pragma hdrstop

SLAPI PPTimeSeries::PPTimeSeries() : Tag(PPOBJ_TIMESERIES), ID(0), Flags(0)
{
	PTR32(Name)[0] = 0;
	PTR32(Symb)[0] = 0;
	memzero(Reserve, sizeof(Reserve));
	Reserve2[0] = 0;
	Reserve2[1] = 0;
}

int FASTCALL PPTimeSeries::IsEqual(const PPTimeSeries & rS) const
{
	int    eq = 1;
	if(!sstreq(Name, rS.Name))
		eq = 0;
	else if(!sstreq(Symb, rS.Symb))
		eq = 0;
	else if(Flags != rS.Flags)
		eq = 0;
	else if(BuyMarg != rS.BuyMarg)
		eq = 0;
	else if(SellMarg != rS.SellMarg)
		eq = 0;
	else if(Prec != rS.Prec)
		eq = 0;
	else if(!sstreq(CurrencySymb, rS.CurrencySymb))
		eq = 0;
	return eq;	
}

SLAPI PPObjTimeSeries::PPObjTimeSeries(void * extraPtr) : PPObjReference(PPOBJ_TIMESERIES, extraPtr)
{
}

int SLAPI PPObjTimeSeries::EditDialog(PPTimeSeries * pEntry)
{
	class TimeSeriesDialog : public TDialog {
	public:
		TimeSeriesDialog() : TDialog(DLG_TIMSER)
		{
		}
		int    setDTS(const PPTimeSeries * pData)
		{
			RVALUEPTR(Data, pData);
			int    ok = 1;
			SString temp_buf;
			setCtrlLong(CTL_TIMSER_ID, Data.ID);
			setCtrlString(CTL_TIMSER_NAME, temp_buf = Data.Name);
			setCtrlString(CTL_TIMSER_SYMB, temp_buf = Data.Symb);
			AddClusterAssoc(CTL_TIMSER_FLAGS, 0, PPCommObjEntry::fPassive);
			SetClusterData(CTL_TIMSER_FLAGS, Data.Flags);
			disableCtrl(CTL_TIMSER_ID, (!PPMaster || Data.ID));
			return ok;
		}
		int    getDTS(PPTimeSeries * pData)
		{
			int    ok = 1;
			uint   sel = 0;
			PPID   _id = Data.ID;
			SString temp_buf;
			getCtrlData(CTL_TIMSER_ID, &_id);
			getCtrlString(sel = CTL_TIMSER_NAME, temp_buf);
			THROW(Obj.CheckName(_id, temp_buf, 1));
			STRNSCPY(Data.Name, temp_buf);
			getCtrlString(sel = CTL_TIMSER_SYMB, temp_buf);
			THROW(Obj.ref->CheckUniqueSymb(Obj.Obj, _id, temp_buf, offsetof(ReferenceTbl::Rec, Symb)));
			STRNSCPY(Data.Symb, temp_buf);
			GetClusterData(CTL_TIMSER_FLAGS, &Data.Flags);
			ASSIGN_PTR(pData, Data);
			CATCH
				ok = PPErrorByDialog(this, sel);
			ENDCATCH
			return ok;
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(event.isCmd(cmTimSerStat)) {
				SString temp_buf;
				SString out_buf;
				if(Data.ID) {
					STimeSeries ts;
					if(Obj.GetTimeSeries(Data.ID, ts) > 0) {
						const uint tsc = ts.GetCount();
						if(tsc) {
							STimeSeries::Stat stat(0);
							//ts.Sort(); // @debug
							ts.Analyze("close", stat);
							out_buf.CatEq("count", stat.GetCount()).Space().CRB();
							out_buf.CatEq("min", stat.GetMin(), MKSFMTD(0, 5, 0)).Space().CRB();
							out_buf.CatEq("max", stat.GetMax(), MKSFMTD(0, 5, 0)).Space().CRB();
							out_buf.CatEq("avg", stat.GetExp(), MKSFMTD(0, 5, 0)).Space().CRB();
							out_buf.CatEq("delta-avg", stat.DeltaAvg, MKSFMTD(0, 8, 0)).Space().CRB();
							out_buf.Cat((stat.State & stat.stSorted) ? "sorted" : "unsorted").Space().CRB();
							out_buf.Cat((stat.State & stat.stHasTmDup) ? "has dup" : "no dup").Space().CRB();
							SUniTime utm;
							LDATETIME dtm;
							ts.GetTime(0, &utm);
							utm.Get(dtm);
							out_buf.CatEq("first_time", temp_buf.Z().Cat(dtm, DATF_DMY|DATF_CENTURY, TIMF_HMS)).Space().CRB();
							ts.GetTime(tsc-1, &utm);
							utm.Get(dtm);
							out_buf.CatEq("last_time", temp_buf.Z().Cat(dtm, DATF_DMY|DATF_CENTURY, TIMF_HMS)).Space().CRB();
							//
#if 0 // @experimental {
							if(tsc >= 1000) {
								RealArray frame;
								StatBase sb;
								double max_vlct_mn = 0.0;
								uint   max_vlct_mn_frame_size = 0;
								double max_vlct_mx = 0.0;
								uint   max_vlct_mx_frame_size = 0;
								for(uint frame_size = 5; frame_size <= 120; frame_size++) {
									StatBase fsb_mn;
									StatBase fsb_mx;
									for(uint fs = 0; fs < (tsc-frame_size); fs++) {
										ts.GetFrame("close", fs, frame_size, STimeSeries::nfOne|STimeSeries::nfBaseStart, frame);
										if(frame.getCount() == frame_size) {
											sb.Init(0);
											for(uint i = 0; i < frame.getCount(); i++) {
												sb.Step(frame.at(i));
											}
											sb.Finish();
											SUniTime utm_f_beg;
											SUniTime utm_f_end;
											LDATETIME dtm_beg;
											LDATETIME dtm_end;
											ts.GetTime(fs, &utm_f_beg);
											ts.GetTime(fs+frame_size-1, &utm_f_end);
											utm_f_beg.Get(dtm_beg);
											utm_f_end.Get(dtm_end);
											long sec = diffdatetimesec(dtm_end, dtm_beg) + 60;
											double _mn = sb.GetMin();
											double _mx = sb.GetMax();
											fsb_mn.Step(_mn / sec);
											fsb_mx.Step(_mx / sec);
										}
									}
									fsb_mn.Finish();
									fsb_mx.Finish();									
									if(fsb_mn.GetExp() > max_vlct_mn) {
										max_vlct_mn = fsb_mn.GetExp();
										max_vlct_mn_frame_size = frame_size;
									}
									if(fsb_mx.GetExp() > max_vlct_mx) {
										max_vlct_mx = fsb_mx.GetExp();
										max_vlct_mx_frame_size = frame_size;
									}
								}
								out_buf.CatEq("max_vlct_mx_frame_size", max_vlct_mx_frame_size).Space().CRB();
								out_buf.CatEq("max_vlct_mx", max_vlct_mx).Space().CRB();
								out_buf.CatEq("max_vlct_mn_frame_size", max_vlct_mx_frame_size).Space().CRB();
								out_buf.CatEq("max_vlct_mn", max_vlct_mx).Space().CRB();
							}
#endif // } 0 @experimental
						}
						else
							out_buf = "empty series";
					}
					else
						out_buf = "no series";
				}
				else
					out_buf = "no series";
				setCtrlString(CTL_TIMSER_STAT, out_buf);
			}
			else
				return;
			clearEvent(event);
		}
		PPObjTimeSeries Obj;
		PPTimeSeries Data;
	};
	int    ok = -1;
	TimeSeriesDialog * p_dlg = 0;
	if(pEntry) {
		SString obj_title;
		THROW(CheckDialogPtr(&(p_dlg = new TimeSeriesDialog())));
		THROW(EditPrereq(&pEntry->ID, p_dlg, 0));
		p_dlg->setTitle(GetObjectTitle(Obj, obj_title));
		p_dlg->setDTS(pEntry);
		while(ok < 0 && ExecView(p_dlg) == cmOK) {
			if(p_dlg->getDTS(pEntry)) {
				ok = 1;
			}
		}
	}
	CATCHZOK
	delete p_dlg;
	return ok;
}

//virtual 
int SLAPI PPObjTimeSeries::Edit(PPID * pID, void * extraPtr)
{
	int    ok = cmCancel;
	int    is_new = 0;
	PPTimeSeries rec;
	THROW(EditPrereq(pID, 0, &is_new));
	MEMSZERO(rec);
	if(!is_new) {
		THROW(Search(*pID, &rec) > 0);
	}
	{
		THROW(ok = EditDialog(&rec));
		if(ok > 0) {
			THROW(is_new || CheckRights(PPR_MOD));
			if(*pID)
				*pID = rec.ID;
			THROW(PutPacket(pID, &rec, 1));
			ok = cmOK;
		}
	}
	CATCHZOKPPERR
	return ok;
}

int  SLAPI PPObjTimeSeries::RemoveObjV(PPID id, ObjCollection * pObjColl, uint options/* = rmv_default*/, void * pExtraParam)
{
	int    r = -1, conf = 1;
	THROW(CheckRights(PPR_DEL));
	if(options & PPObject::user_request) {
		conf = CONFIRM(PPCFM_DELETE);
	}
	if(conf) {
		PPWait(1);
		THROW(PutPacket(&id, 0, BIN(options & PPObject::use_transaction)));
		r = 1;
	}
	CATCH
		r = 0;
		if(options & PPObject::user_request)
			PPError();
	ENDCATCH
	PPWait(0);
	return r;
}

int SLAPI PPObjTimeSeries::PutPacket(PPID * pID, PPTimeSeries * pPack, int use_ta)
{
	int    ok = 1;
	PPID   _id = DEREFPTRORZ(pID);
	const  int is_new = (_id == 0);
	const  int is_removing = BIN(*pID != 0 && pPack == 0);
	PPID   hid = 0;
	PPTimeSeries org_pack;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(_id) {
			THROW(GetPacket(_id, &org_pack) > 0);
		}
		if(pPack == 0) {
			if(*pID) {
				TextRefIdent tri(Obj, _id, PPTRPROP_TIMESERIES);
				THROW(CheckRights(PPR_DEL));
				THROW(ref->RemoveItem(Obj, _id, 0));
				THROW(ref->RemoveProperty(Obj, _id, 0, 0));
				//THROW(ref->Ot.PutList(Obj, _id, 0, 0));
				THROW(ref->UtrC.SetTimeSeries(tri, 0, 0));
				//THROW(RemoveSync(_id));
				DS.LogAction(PPACN_OBJRMV, Obj, *pID, hid, 0);
			}
		}
		else {
			THROW(CheckRightsModByID(pID));
			if(_id) {
				if(pPack->IsEqual(org_pack))
					ok = -1;
				else {
					THROW(ref->UpdateItem(Obj, _id, pPack, 1, 0));
					//THROW(ref->Ot.PutList(Obj, _id, &pPack->TagL, 0));
					DS.LogAction(PPACN_OBJUPD, Obj, _id, 0, 0);
				}
			}
			else {
				THROW(ref->AddItem(Obj, &_id, pPack, 0));
				pPack->ID = _id;
				//THROW(ref->Ot.PutList(Obj, _id, &pPack->TagL, 0));
				DS.LogAction(PPACN_OBJADD, Obj, _id, 0, 0);
				ASSIGN_PTR(pID, _id);
			}
		}
		THROW(tra.Commit());
	}
	CATCH
		if(is_new) {
			*pID = 0;
			if(pPack)
				pPack->ID = 0;
		}
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI PPObjTimeSeries::GetPacket(PPID id, PPTimeSeries * pPack)
{
	return Search(id, pPack);
}

int SLAPI PPObjTimeSeries::SetTimeSeries(PPID id, STimeSeries * pTs, int use_ta)
{
	int    ok = 1;
	PPTimeSeries ts_rec;
	{
		TextRefIdent tri(Obj, id, PPTRPROP_TIMESERIES);
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(Search(id, &ts_rec) > 0);
		THROW(PPRef->UtrC.SetTimeSeries(tri, pTs, 0));
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI PPObjTimeSeries::GetTimeSeries(PPID id, STimeSeries & rTs)
{
	TextRefIdent tri(Obj, id, PPTRPROP_TIMESERIES);
	return PPRef->UtrC.Search(tri, rTs);
}

int SLAPI PPObjTimeSeries::Browse(void * extraPtr)
{
	class TsViewDialog : public ObjViewDialog {
	public:
		TsViewDialog(PPObjTimeSeries * pObj) : ObjViewDialog(DLG_TSVIEW, pObj, 0)
		{
		}
	private:
		virtual void extraProc(long id)
		{
			PPObjTimeSeries * p_obj = (PPObjTimeSeries *)P_Obj;
			if(p_obj) {
				//p_obj->Test();
				//p_obj->AnalyzeTsTradeFrames();
				//p_obj->AnalyzeTsAftershocks();
				p_obj->AnalyzeStrategies();
			}
		}
	};
	//return RefObjView(this, 0, 0);
	//int SLAPI RefObjView(PPObject * pObj, long charryID, void * extraPtr)
	{
		int    ok = 1;
		TDialog * dlg = new TsViewDialog(this);
		if(CheckDialogPtrErr(&dlg))
			ExecViewAndDestroy(dlg);
		else
			ok = 0;
		return ok;
	}
}

int SLAPI PPObjTimeSeries::Test() // @experimental
{
	int    ok = 1;
	SString temp_buf;
	SString src_file_name;
	SString test_file_name;
	SLS.QueryPath("testroot", src_file_name);
	src_file_name.SetLastSlash().Cat("data").SetLastSlash().Cat("ts").SetLastSlash().Cat("test-symb-export-eurusd.csv");
	SFile f_in(src_file_name, SFile::mRead);
	if(f_in.IsValid()) {
		test_file_name = src_file_name;
		SPathStruc::ReplaceExt(test_file_name, "out", 1);

		SString line_buf;
		StringSet ss_in(",");
		STimeSeries ts;

		LDATETIME dtm;
		double open = 0.0;
		double close = 0.0;
		long   tick_vol = 0;
		long   real_vol = 0;
		long   spread = 0;

		//uint   vecidx_open = 0;
		uint   vecidx_close = 0;
		//uint   vecidx_ticvol = 0;
		uint   vecidx_realvol = 0;
		//uint   vecidx_spread = 0;
		//THROW_SL(ts.AddValueVec("open", T_DOUBLE, 0, &vecidx_open));
		//THROW_SL(ts.AddValueVec("open", T_INT32, 5, &vecidx_open));
		//THROW_SL(ts.AddValueVec("close", T_DOUBLE, 0, &vecidx_close));
		THROW_SL(ts.AddValueVec("close", T_INT32, 5, &vecidx_close));
		//THROW_SL(ts.AddValueVec("tick_volume", T_INT32, 0, &vecidx_ticvol));
		THROW_SL(ts.AddValueVec("real_volume", T_INT32, 0, &vecidx_realvol));
		//THROW_SL(ts.AddValueVec("spread", T_INT32, 0, &vecidx_spread));
		{
			uint8 sign[8];
			size_t actual_size = 0;
			if(f_in.Read(sign, 4, &actual_size) && actual_size == 4) {
				if(sign[0] == 0xEF && sign[1] == 0xBB && sign[2] == 0xBF)
					f_in.Seek(3);
				else
					f_in.Seek(0);
			}
		}
		while(f_in.ReadLine(line_buf)) {
			line_buf.Chomp().Strip();
			if(line_buf.NotEmpty()) {
				ss_in.setBuf(line_buf);
				dtm.Z();
				open = 0.0;
				close = 0.0;
				tick_vol = 0;
				real_vol = 0;
				spread = 0;
				for(uint ssp = 0, fldn = 0; ss_in.get(&ssp, temp_buf); fldn++) {
					switch(fldn) {
						case 0: strtodate(temp_buf, DATF_YMD, &dtm.d); break;
						case 1: strtotime(temp_buf, TIMF_HMS, &dtm.t); break;
						case 2: open = temp_buf.ToReal(); break;
						case 3: close = temp_buf.ToReal(); break;
						case 4: tick_vol = temp_buf.ToLong(); break;
						case 5: real_vol = temp_buf.ToLong(); break;
						case 6: spread = temp_buf.ToLong(); break;
					}
				}
				if(checkdate(&dtm) && close > 0.0) {
					SUniTime ut;
					ut.Set(dtm, SUniTime::indMin);
					uint   item_idx = 0;
					THROW_SL(ts.AddItem(ut, &item_idx));
					//THROW_SL(ts.SetValue(item_idx, vecidx_open, open));
					THROW_SL(ts.SetValue(item_idx, vecidx_close, close));
					//THROW_SL(ts.SetValue(item_idx, vecidx_ticvol, tick_vol));
					THROW_SL(ts.SetValue(item_idx, vecidx_realvol, real_vol));
					//THROW_SL(ts.SetValue(item_idx, vecidx_spread, spread));
				}
			}
		}
		{
			//SFile f_out(test_file_name, SFile::mWrite);
			//
			{
				STimeSeries dts;
				SBuffer sbuf; // serialize buf
				SBuffer cbuf; // compress buf
				SBuffer dbuf; // decompress buf
				SSerializeContext sctx;
				THROW_SL(ts.Serialize(+1, sbuf, &sctx));
				{
					{
						SCompressor c(SCompressor::tZLib);
						THROW_SL(c.CompressBlock(sbuf.GetBuf(sbuf.GetRdOffs()), sbuf.GetAvailableSize(), cbuf, 0, 0));
					}
					{
						SCompressor c(SCompressor::tZLib);
						THROW_SL(c.DecompressBlock(cbuf.GetBuf(cbuf.GetRdOffs()), cbuf.GetAvailableSize(), dbuf));
					}
					THROW_SL(dts.Serialize(-1, dbuf, &sctx));
				}
				dts.GetCount();
				THROW(dts.Sort());
				{
					SFile f_out(test_file_name, SFile::mWrite);
					THROW(f_out.IsValid());
					for(uint i = 0; i < dts.GetCount(); i++) {
						SUniTime ut;
						dts.GetTime(i, &ut);
						ut.Get(dtm);
						//THROW(dts.GetValue(i, vecidx_open, &open));
						THROW(dts.GetValue(i, vecidx_close, &close));
						//THROW(dts.GetValue(i, vecidx_ticvol, &tick_vol));
						THROW(dts.GetValue(i, vecidx_realvol, &real_vol));
						//THROW(dts.GetValue(i, vecidx_spread, &spread));
						line_buf.Z().Cat(dtm.d, DATF_ANSI|DATF_CENTURY).Comma().Cat(dtm.t, TIMF_HM).Comma().
							/*Cat(open, MKSFMTD(0, 5, 0)).Comma().*/Cat(close, MKSFMTD(0, 5, 0)).Comma()./*Cat(tick_vol).Comma().*/Cat(real_vol)/*.Comma().Cat(spread)*/.CR();
						THROW(f_out.WriteLine(line_buf));
					}
				}
			}
			{
				PPID    id = 0;
				PPTimeSeries ts_rec;
				STimeSeries ex_ts;
				int gts = 0;
				if(SearchBySymb("eurusd", &id, &ts_rec) > 0) {
					gts = GetTimeSeries(id, ex_ts);
				}
				else {
					MEMSZERO(ts_rec);
					STRNSCPY(ts_rec.Name, "eurusd");
					STRNSCPY(ts_rec.Symb, "eurusd");
					THROW(PutPacket(&id, &ts_rec, 1));
				}
				THROW(SetTimeSeries(id, &ts, 1));
			}
		}
	}
	CATCHZOK
	return ok;
}

static int SLAPI AnalyzeTsTradeFrame(const STimeSeries & rTs, uint frameSize, double target, double * pFreq, double * pMaxDuckAvg)
{
	int    ok = -1;
	const  uint tsc = rTs.GetCount();
	RealArray frame;
	uint   frame_count = 0;
	uint   target_count = 0;
	StatBase max_duck_stat;
	const double zero = 0.0;
	for(uint fs = 0; fs < (tsc-frameSize); fs++) {
		rTs.GetFrame("close", fs, frameSize, STimeSeries::nfOne|STimeSeries::nfBaseStart, frame);
		int   target_reached = 0;
		double max_duck = 1.0;
		if(frame.getCount() == frameSize) {
			for(uint i = 0; !target_reached && i < frame.getCount(); i++) {
				const double fv = frame.at(i);
				SETMIN(max_duck, fv);
				if(fv > target)
					target_reached = 1;
			}
			frame_count++;
			ok = 1;
		}
		if(target_reached) {
			max_duck_stat.Step(max_duck);
			target_count++;
		}
	}
	max_duck_stat.Finish();
	ASSIGN_PTR(pFreq, ((double)target_count) / ((double)frame_count));
	ASSIGN_PTR(pMaxDuckAvg, max_duck_stat.GetExp());
	return ok;
}

int SLAPI PPObjTimeSeries::AnalyzeTsAftershocks()
{
	int    ok = 1;
	PPIDArray id_list;
	PPTimeSeries ts_rec;
	for(SEnum en = Enum(0); en.Next(&ts_rec) > 0;) {
		id_list.add(ts_rec.ID);
	}
	if(id_list.getCount()) {
		id_list.sortAndUndup();
		for(uint i = 0; i < id_list.getCount(); i++) {
			const PPID id = id_list.get(i);
			if(Search(id, &ts_rec) > 0) {
				STimeSeries ts;
				if(GetTimeSeries(id, ts) > 0) {
					TrainNnParam tnnp(ts_rec, TrainNnParam::fAnalyzeFrame);
					AnalyzeAftershock(ts, tnnp);
				}
			}
		}
	}
	return ok;
}

int SLAPI PPObjTimeSeries::AnalyzeStrategies()
{
	int    ok = 1;
	static const char * pp_symbols[] = { "eurusd", /*"USDJPY", "USDCHF", "GBPUSD", "AUDUSD", "EURCAD", "GBPJPY"*/ };
	PPIDArray id_list;
	PPTimeSeries ts_rec;
	for(SEnum en = Enum(0); en.Next(&ts_rec) > 0;) {
		if(pp_symbols) {
			for(uint i = 0; i < SIZEOFARRAY(pp_symbols); i++) {
				if(sstreqi_ascii(ts_rec.Symb, pp_symbols[i])) {
					id_list.add(ts_rec.ID);
					break;
				}
			}
		}
		else
			id_list.add(ts_rec.ID);
	}
	if(id_list.getCount()) {
		id_list.sortAndUndup();
		for(uint i = 0; i < id_list.getCount(); i++) {
			const PPID id = id_list.get(i);
			if(Search(id, &ts_rec) > 0) {
				STimeSeries ts;
				if(GetTimeSeries(id, ts) > 0) {
					STimeSeries::Stat st(0);
					ts.Analyze("close", st);
					TrainNnParam tnnp(ts_rec, TrainNnParam::fAnalyzeFrame);
					tnnp.SpikeQuant = st.DeltaAvg / 2.0;
					tnnp.TargetQuant = 0;
					tnnp.TrendGe = 8E-6;
					tnnp.InputFrameSize = 2000;
					//tnnp.AppendSeq(1, _GE_);
					//tnnp.AppendSeq(1, _GE_);
					//tnnp.AppendSeq(0, _GE_);
					tnnp.MaxDuckQuant = 20;
					AnalyzeStrategy(ts, tnnp);
				}
			}
		}
	}
	return ok;
}

int SLAPI PPObjTimeSeries::AnalyzeTsTradeFrames()
{
	static const char * pp_symbols[] = { "eurusd", /*"USDJPY", "USDCHF",*/ "GBPUSD", "AUDUSD"/*, "EURCAD", "GBPJPY"*/ };
	int    ok = 1;
	PPIDArray id_list;
	PPTimeSeries ts_rec;
	for(SEnum en = Enum(0); en.Next(&ts_rec) > 0;) {
		if(pp_symbols) {
			for(uint i = 0; i < SIZEOFARRAY(pp_symbols); i++) {
				if(sstreqi_ascii(ts_rec.Symb, pp_symbols[i])) {
					id_list.add(ts_rec.ID);
					break;
				}
			}
		}
		else
			id_list.add(ts_rec.ID);
	}
	if(id_list.getCount()) {
		id_list.sortAndUndup();
		for(uint i = 0; i < id_list.getCount(); i++) {
			const PPID id = id_list.get(i);
			if(Search(id, &ts_rec) > 0) {
				STimeSeries ts;
				if(GetTimeSeries(id, ts) > 0) {
					TrainNnParam tnnp(ts_rec, TrainNnParam::fAnalyzeFrame);
					tnnp.InputFrameSize = 500;
					static const uint frame_size_list[] = { 5, 10, 15, 20 };
					static const double target_list[] = { 0.0, 0.10, 0.25, 0.5, 1.0, 1.25, 1.5 };
					for(uint fi = 0; fi < SIZEOFARRAY(frame_size_list); fi++) {
						tnnp.ForwardFrameSize = frame_size_list[fi];
						for(uint ti = 0; ti < SIZEOFARRAY(target_list); ti++) {
							const double target = target_list[ti];
							tnnp.Target = target;
							const uint duck_part_count = 8;
							if(target == 0.0) {
								tnnp.MaxDuck = 0.0;
								ProcessNN(ts, tnnp);
							}
							else {
								for(uint dp = 0; dp <= duck_part_count; dp++) {
									tnnp.MaxDuck = dp * target / duck_part_count;
									ProcessNN(ts, tnnp);
								}
							}
						}
					}
				}
			}
		}
	}
	return ok;
}

int SLAPI PPObjTimeSeries::TrainNN()
{
	int   ok = 1;
	PPID  id = 0;
	PPTimeSeries ts_rec;
	if(SearchBySymb("eurusd", &id, &ts_rec) > 0) {
		STimeSeries ts;
		TrainNnParam tnnp(ts_rec.Symb, TrainNnParam::fTest|TrainNnParam::fTeach);
		tnnp.Margin = 0.005;
		tnnp.ForwardFrameSize = 20;
		/*
		tnnp.Target = 1;
		tnnp.MaxDuck = 0.25;
		*/
		tnnp.Target = 0.5;
		tnnp.MaxDuck = 0.25;

		tnnp.InputFrameSize = 500;
		tnnp.HiddenLayerDim = 9000;
		tnnp.LearningRate = 0.25f;
		tnnp.EpochCount = 2;
		if(GetTimeSeries(id, ts) > 0) {
			ok = ProcessNN(ts, tnnp/*20, 1.001, 0.9998*/);
		}
	}
	return ok;
}

#include <fann2.h>

SLAPI PPObjTimeSeries::TrainNnParam::TrainNnParam(const char * pSymb, long flags) : Symb(pSymb), Flags(flags), Margin(0.0), ForwardFrameSize(0), Target(0.0), 
	MaxDuck(0.0), InputFrameSize(0), HiddenLayerDim(0), EpochCount(1), LearningRate(0.1f),
	SpikeQuant(0.0), TrendGe(0.0), SeqCount(0), TargetQuant(0), MaxDuckQuant(0)
{
	memzero(Seq, sizeof(Seq));
}

SLAPI PPObjTimeSeries::TrainNnParam::TrainNnParam(const PPTimeSeries & rTsRec, long flags) : Symb(rTsRec.Symb), Flags(flags), Margin(rTsRec.BuyMarg), ForwardFrameSize(0), Target(0.0), 
	MaxDuck(0.0), InputFrameSize(0), HiddenLayerDim(0), EpochCount(1), LearningRate(0.1f),
	SpikeQuant(0.0), TrendGe(0.0), SeqCount(0), TargetQuant(0), MaxDuckQuant(0)
{
	memzero(Seq, sizeof(Seq));
}

void SLAPI PPObjTimeSeries::TrainNnParam::Reset()
{
	SpikeQuant = 0.0;
	TrendGe = 0.0;
	MaxDuckQuant = 0;
	SeqCount = 0;
}

int SLAPI PPObjTimeSeries::TrainNnParam::AppendSeq(int quantCount, int condition)
{
	if(SeqCount < SIZEOFARRAY(Seq)) {
		Seq[SeqCount++] = (((int16)condition) << 16) | ((int16)quantCount);
		return 1;
	}
	else
		return 0;
}

int SLAPI PPObjTimeSeries::TrainNnParam::GetSeq(uint seqIdx, int * pQuantCount, int * pCondition) const
{
	if(seqIdx < SeqCount && seqIdx < SIZEOFARRAY(Seq)) {
		ASSIGN_PTR(pQuantCount, (int)(Seq[seqIdx] & 0x0000ffff));
		ASSIGN_PTR(pCondition, (int)((Seq[seqIdx] & 0xffff0000) >> 16));
		return 1;
	}
	else
		return 0;
}

SString & SLAPI PPObjTimeSeries::TrainNnParam::MakeFileName(SString & rBuf) const
{
	return rBuf.Z().Cat(Symb).CatChar('-').Cat(ForwardFrameSize).CatChar('-').Cat((long)R0(Target * 1000.0)).
		CatChar('-').Cat((long)R0(MaxDuck * 1000.0)).Dot().Cat("fann").ToLower();
}

struct __TsProbEntry {
	uint   MeanMult;
	uint   Duration;

	uint   CaseCount;
	uint   WinCount;
	uint   LossCount;
	double WinSum;
	double LossSum;
};

int SLAPI PPObjTimeSeries::IsCase(const STimeSeries & rTs, const TrainNnParam & rP, uint vecIdx, uint lastIdx) const
{
	int    ok = -1;
	const  uint tsc = rTs.GetCount();
	const  uint seqc = rP.SeqCount;
	if(lastIdx < tsc && lastIdx > seqc && rP.SpikeQuant > 0.0) {
		double seq_values[SIZEOFARRAY(rP.Seq)+1];
		{
			for(uint i = (lastIdx-seqc-1), svidx = 0; i <= lastIdx; i++, svidx++)
				rTs.GetValue(i, vecIdx, &seq_values[svidx]);
		}
		double prev_value = seq_values[0];
		bool   qc = true;
		for(uint i = 1; qc && i <= seqc; i++) {
			int quant_count;
			int condition;
			assert(rP.GetSeq(i-1, &quant_count, &condition));
			const double value = seq_values[i];
			const double delta = ((value - prev_value) / prev_value);
			const long rdelta = (long)R0(delta);
			const long rquant = (long)R0(rP.SpikeQuant * quant_count);
			switch(condition) {
				case _GT_: qc = (rdelta > rquant); break;
				case _GE_: qc = (rdelta >= rquant); break;
				case _EQ_: qc = (rdelta == rquant); break;
				case _LT_: qc = (rdelta < rquant); break;
				case _LE_: qc = (rdelta <= rquant); break;
				default: qc = false;
			}
			prev_value = value;
		}
		ok = BIN(qc);
	}
	return ok;
}

int SLAPI PPObjTimeSeries::AnalyzeStrategy(const STimeSeries & rTs, const TrainNnParam & rP)
{
	int    ok = 1;
	uint   vec_idx = 0;
	SString temp_buf;
	SString out_file_name;
	SString stake_out_file_name;
	SString msg_buf;
	//const  uint lss_dist = rP.BackSurvey; // Дистанция для отсчета назад с целью вычисления тренда
	const  uint tsc = rTs.GetCount();
	const  uint seqc = rP.SeqCount;
	if(tsc > (1+rP.InputFrameSize)) {
		LVect lss_vect_x(rP.InputFrameSize);
		LVect lss_vect_y(rP.InputFrameSize);
		lss_vect_x.FillWithSequence(1.0, 1.0); // Заполняем ось ординат последовательными значениями 1.0..lss_dim
		THROW_SL(rTs.GetValueVecIndex("close", &vec_idx));
		{
			struct ResultEntry {
				SString & MakeResultHeader(SString & rBuf) const
				{
					rBuf.Z().Cat("symb").
						Tab().Cat("time").
						Tab().Cat("close").
						Tab().Cat("delta").
						Tab().Cat("trend").
						Tab().Cat("seq").
						Tab().Cat("stake").
						Tab().Cat("stake-time").
						Tab().Cat("win").
						Tab().Cat("loss");
					return rBuf;
				}
				SString & MakeResultLine(const SString & rSymb, SString & rBuf) const
				{
					rBuf.Z().Cat(rSymb).
						Tab().Cat(Tm, DATF_ISO8601|DATF_CENTURY, 0).
						Tab().Cat(Value, MKSFMTD(12,  5, 0)).
						Tab().Cat(Delta, MKSFMTD(25, 20, 0)).
						Tab().Cat(Trend, MKSFMTD(15, 10, 0)).
						Tab().Cat(SeqTxt).
						Tab().Cat(Stake, MKSFMTD(10,  5, 0)).
						Tab().Cat(StakeTmCount).
						Tab().Cat(Win,   MKSFMTD(8, 5, 0)).
						Tab().Cat(Loss,  MKSFMTD(8, 5, 0));
					return rBuf;
				}
				ResultEntry()
				{
					Tm.Z();
					PrevValue = 0.0;
					Value = 0.0;
					Delta = 0.0;
					Trend = 0.0;
					SeqTxt.Z();
					Stake = 0.0;
					StakeCount = 0;
					StakeTmCount = 0;
					StakeSec = 0;
					WinCount = 0;
					LossCount = 0;
					Win = 0.0;
					Loss = 0.0;
					WinTotal = 0.0;
					LossTotal = 0.0;
				}
				void Reset()
				{
					Tm.Z();
					Delta = 0.0;
					Trend = 0.0;
					SeqTxt.Z();
					Win = 0.0;
					Loss = 0.0;
					Stake = 0.0;
					StakeSec = 0;
					StakeTmCount = 0;
				}
				LDATETIME Tm;
				double PrevValue;
				double Value;
				double Delta;
				double Trend;
				SString SeqTxt;
				double Stake;
				uint   StakeCount;
				uint   StakeTmCount;
				uint   StakeSec;
				uint   WinCount;
				uint   LossCount;
				double Win;
				double Loss;
				double WinTotal;
				double LossTotal;
			};
			ResultEntry result_entry;
			PPGetFilePath(PPPATH_OUT, "AnalyzeTsStrategy.txt", out_file_name);
			PPGetFilePath(PPPATH_OUT, "AnalyzeTsStrategy-stake.txt", stake_out_file_name);
			const int is_out_file_exists = fileExists(out_file_name);
			const int is_stake_out_file_exists = fileExists(stake_out_file_name);
			SFile f_out(out_file_name, SFile::mAppend);
			SFile f_stake_out(stake_out_file_name, SFile::mAppend);
			if(!is_out_file_exists) {
				msg_buf.Z().Cat(getcurdatetime_(), DATF_YMD|DATF_CENTURY, TIMF_HMS);
				f_out.WriteLine(msg_buf.CR());
				f_out.WriteLine(result_entry.MakeResultHeader(msg_buf).CR());
			}
			if(!is_stake_out_file_exists) {
				msg_buf.Z().Cat(getcurdatetime_(), DATF_YMD|DATF_CENTURY, TIMF_HMS);
				f_stake_out.WriteLine(msg_buf.CR());
				f_stake_out.WriteLine(result_entry.MakeResultHeader(msg_buf).CR());
			}
			RealArray trend_vect;
			SUniTime ut;
			rTs.GetTime(0, &ut);
			ut.Get(result_entry.Tm);
			rTs.GetValue(0, vec_idx, &result_entry.Value);
			f_out.WriteLine(result_entry.MakeResultLine(rTs.GetSymb(), msg_buf).CR());
			f_stake_out.WriteLine(result_entry.MakeResultLine(rTs.GetSymb(), msg_buf).CR());
			result_entry.PrevValue = result_entry.Value;
			for(uint i = 1; i < tsc; i++) {
				rTs.GetTime(i, &ut);
				ut.Get(result_entry.Tm);
				rTs.GetValue(i, vec_idx, &result_entry.Value);
				result_entry.Delta = ((result_entry.Value - result_entry.PrevValue) / result_entry.PrevValue);
				result_entry.SeqTxt.Z();
				const uint __seqc = 5;
				if(__seqc && i > __seqc && rP.SpikeQuant > 0.0) {
					double seq_values[SIZEOFARRAY(rP.Seq)+1];
					{
						for(uint j = (i-__seqc-1), svidx = 0; j <= i; j++, svidx++)
							rTs.GetValue(j, vec_idx, &seq_values[svidx]);
					}
					double local_prev_value = seq_values[0];
					for(uint j = 1; j <= __seqc; j++) {
						const double local_value = seq_values[j];
						const double local_delta = ((local_value - local_prev_value) / local_prev_value);
						if(j > 1)
							result_entry.SeqTxt.Space();
						long qd = (long)R0(local_delta / rP.SpikeQuant);
						if(qd > 0)
							result_entry.SeqTxt.CatChar('+');
						else if(qd == 0)
							result_entry.SeqTxt.Space();
						result_entry.SeqTxt.Cat(qd);
						local_prev_value = local_value;
					}
				}
				if(i >= rP.InputFrameSize) {
					LssLin lss;
					const int gvr = rTs.GetLVect(vec_idx, i-rP.InputFrameSize, lss_vect_y);
					assert(gvr > 0);
					lss.Solve(lss_vect_x, lss_vect_y);
					result_entry.Trend = lss.B;
					if(rP.TrendGe == 0.0 || result_entry.Trend >= rP.TrendGe) {
						const int icr = IsCase(rTs, rP, vec_idx, i);
						if(icr > 0) {
							result_entry.StakeCount++;
							result_entry.Stake = 1.0;
							double local_result = 0.0; 
							double prev_value;
							rTs.GetValue(i, vec_idx, &prev_value);
							const double stake_value = prev_value;
							double peak = prev_value;
							uint   max_duck_quant = rP.MaxDuckQuant;
							uint   max_duck_quant_limit = MIN(rP.MaxDuckQuant, 1);
							{
								uint k = i+1;
								while(k < tsc) {
									double value;
									rTs.GetValue(k, vec_idx, &value);
									const double delta = ((value - prev_value) / prev_value);
									const double peak_delta = ((value - peak) / peak);
									const double stake_delta = ((value - stake_value) / stake_value);
									local_result = stake_delta / rP.Margin;
									if(peak_delta < 0.0 && (peak_delta < -(max_duck_quant * rP.SpikeQuant))) {
										break;
									}
									else if(rP.TargetQuant && (stake_delta > (rP.TargetQuant * rP.SpikeQuant))) {
										break;									
									}
									else {
										if(stake_delta > 0.0) {
											long to_dec_max_duck = (long)R0(stake_delta / rP.SpikeQuant);
											if(to_dec_max_duck <= (rP.MaxDuckQuant - max_duck_quant_limit))
												max_duck_quant = rP.MaxDuckQuant - to_dec_max_duck;
											else
												max_duck_quant = max_duck_quant_limit;
										}
										SETMAX(peak, value);
										prev_value = value;
										k++;
									}
								}
								result_entry.StakeTmCount = k - i;
								SUniTime local_ut;
								LDATETIME local_tm;
								rTs.GetTime(k, &local_ut);
								local_ut.Get(local_tm);
								result_entry.StakeSec = diffdatetimesec(local_tm, result_entry.Tm);
							}
							if(local_result > 0.0) {
								result_entry.WinCount++;
								result_entry.Win += local_result;
								result_entry.WinTotal += local_result;
							}
							else if(local_result < 0.0) {
								result_entry.LossCount++;
								result_entry.Loss -= local_result;
								result_entry.LossTotal -= local_result;
							}
						}
					}
				}
				f_out.WriteLine(result_entry.MakeResultLine(rTs.GetSymb(), msg_buf).CR());
				if(result_entry.Stake != 0.0) {
					f_stake_out.WriteLine(result_entry.MakeResultLine(rTs.GetSymb(), msg_buf).CR());
				}
				result_entry.Reset();
				result_entry.PrevValue = result_entry.Value;
			}
			{
				msg_buf.Z().
					Tab().Cat(result_entry.WinCount).
					Tab().Cat(result_entry.LossCount).
					Tab().Cat(result_entry.WinTotal,   MKSFMTD(8, 5, ADJ_RIGHT)).
					Tab().Cat(result_entry.LossTotal,  MKSFMTD(8, 5, ADJ_RIGHT));
				f_out.WriteLine(msg_buf.CR());
				f_stake_out.WriteLine(msg_buf.CR());
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPObjTimeSeries::AnalyzeAftershock(const STimeSeries & rTs, const TrainNnParam & rP)
{
	int    ok = 1;
	uint   vec_idx = 0;
	SString temp_buf;
	SString out_file_name;
	SString msg_buf;
	StatBase stat_plus(StatBase::fStoreVals/*|StatBase::fGammaTest|StatBase::fGaussianTest*/);
	StatBase stat_minus(StatBase::fStoreVals/*|StatBase::fGammaTest|StatBase::fGaussianTest*/);
	TSVector <__TsProbEntry> prob_result_list;
	const  uint lss_dist = /*2000*/0; // Дистанция для отсчета назад с целью вычисления тренда
	const  uint tsc = rTs.GetCount();
	LVect lss_vect_x(lss_dist);
	LVect lss_vect_y(lss_dist);
	lss_vect_x.FillWithSequence(1.0, 1.0); // Заполняем ось ординат последовательными значениями 1.0..lss_dim
	if(tsc > (1+lss_dist)) {
		const double margin = (rP.Margin > 0.0) ? rP.Margin : 1.0;
		THROW_SL(rTs.GetValueVecIndex("close", &vec_idx));
		{
			PPGetFilePath(PPPATH_OUT, "AnalyzeTsAfetershock.txt", out_file_name);
			const int is_out_file_exists = fileExists(out_file_name);
			SFile f_out(out_file_name, SFile::mAppend);
			if(!is_out_file_exists) {
				msg_buf.Z().Cat(getcurdatetime_(), DATF_YMD|DATF_CENTURY, TIMF_HMS);
				f_out.WriteLine(msg_buf.CR());
				// symb count stat-plus-max  stat-plus-mean  stat-plus-disp  stat-plus-gauss-test  stat-plus-gamma-test
				msg_buf.Z().Cat("symb").
					Tab().Cat("margin").
					Tab().Cat("count").
					Tab().Cat("sign").
					Tab().Cat("stat-max").
					Tab().Cat("stat-mean").
					Tab().Cat("stat-disp");
				f_out.WriteLine(msg_buf.CR());
			}
			double prev_value;
			rTs.GetValue(0, vec_idx, &prev_value);
			for(uint i = 1; i < tsc; i++) {
				double value;
				rTs.GetValue(i, vec_idx, &value);
				if(prev_value != 0.0) {
					const double delta = ((value - prev_value) / prev_value) / margin;
					if(delta > 0.0)
						stat_plus.Step(delta);
					else if(delta < 0.0)
						stat_minus.Step(-delta);
				}
				prev_value = value;
			}
			stat_plus.Finish();
			stat_minus.Finish();
			// symb count stat-plus-max  stat-plus-mean  stat-plus-disp  stat-plus-gauss-test  stat-plus-gamma-test
			// symb count stat-minus-max stat-minus-mean stat-minus-disp stat-minus-gauss-test stat-minus-gamma-test
			msg_buf.Z().Cat((temp_buf = rP.Symb).Align(12, ADJ_LEFT)).
				Tab().Cat(margin, MKSFMTD(0, 5, 0)).
				Tab().Cat(tsc).
				Tab().Cat("plus").
				Tab().Cat(stat_plus.GetMax(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_plus.GetExp(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_plus.GetStdDev(), MKSFMTD(0, 8, 0));
			f_out.WriteLine(msg_buf.CR());
			msg_buf.Z().Cat((temp_buf = rP.Symb).Align(12, ADJ_LEFT)).
				Tab().Cat(margin, MKSFMTD(0, 5, 0)).
				Tab().Cat(tsc).
				Tab().Cat("minus").
				Tab().Cat(stat_minus.GetMax(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_minus.GetExp(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_minus.GetStdDev(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_minus.GetTestGaussian(), MKSFMTD(0, 8, 0)).
				Tab().Cat(stat_minus.GetTestGamma(), MKSFMTD(0, 8, 0));
			f_out.WriteLine(msg_buf.CR());
			if(1) { 
				//
				// Grid: mean-mult; target; duck; duration
				//
				const double mean_plus = stat_plus.GetExp();
				static const uint   mean_mult_grid[] = { 2, 5, 10, 20, 30, 40, 50 };         // #0
				static const uint   duration_grid[] = { 1, 2, 3, 4, 5, 10, 15, 20, 30 };   // #1
				static const double target_grid[] = { 0.1, 0.5, 1.0, 1.5 };            // #2
				static const double duck_part_grid[] = { 0.1, 0.25, 0.50, 0.75, 1.0 }; // #3

				RealArray trend_vect;
				uint plus_trend_count = 0;
				uint minus_trend_count = 0;
				if(lss_dist) {
					for(uint j = lss_dist+1; j < tsc; j++) {
						assert(j >= lss_dist);
						LssLin lss;
						rTs.GetLVect(vec_idx, j-lss_dist, lss_vect_y);
						lss.Solve(lss_vect_x, lss_vect_y);
						double trend = lss.B;
						if(trend > 0.0) {
							plus_trend_count++;
						}
						else {
							minus_trend_count++;
						}
						THROW_SL(trend_vect.insert(&trend));
					}
				}
				msg_buf.Z().CR().
					Cat("mean-mult").
					Tab().Cat("duration").
					//Tab().Cat("target").
					//Tab().Cat("duck").
					Tab().Cat("case-count").
					Tab().Cat("trend-avg").
					Tab().Cat("win-count").
					Tab().Cat("win-sum").
					Tab().Cat("loss-count").
					Tab().Cat("loss-summ").
					Tab().Cat("win-loss-count").
					Tab().Cat("win-loss-sum");
				f_out.WriteLine(msg_buf.CR());

				for(uint mmi = 0; mmi < SIZEOFARRAY(mean_mult_grid); mmi++) {
					const uint mean_mult = mean_mult_grid[mmi];
					const double threshold = mean_plus * mean_mult;
					for(uint duri = 0; duri < SIZEOFARRAY(duration_grid); duri++) {
						const uint duration = duration_grid[duri];
						//for(uint ti = 0; ti < SIZEOFARRAY(target_grid); ti++) {
							//const double target = target_grid[ti];
							//for(uint dpi = 0; dpi < SIZEOFARRAY(duck_part_grid); dpi++) {
								//const double duck_part = duck_part_grid[dpi];
								rTs.GetValue(lss_dist/*0*/, vec_idx, &prev_value);
								StatBase stat_trend;
								__TsProbEntry prob_result_entry;
								MEMSZERO(prob_result_entry);
								prob_result_entry.MeanMult = mean_mult;
								prob_result_entry.Duration = duration;
								for(uint j = lss_dist+1; j < tsc; j++) {
									double value;
									rTs.GetValue(j, vec_idx, &value);
									const double delta = ((value - prev_value) / prev_value) / margin;
									if(delta >= threshold && (j+duration) < tsc) {
										assert(j >= lss_dist);
										double trend = lss_dist ? trend_vect.at(j-lss_dist-1) : 1.0;
										if(trend > 0.0) {
											stat_trend.Step(trend);
											plus_trend_count++;
											prob_result_entry.CaseCount++;
											double next_value;
											/*for(uint k = 1; k <= duration; k++) {
												rTs.GetValue(j+k, vec_idx, &next_value);
												if(next_value > value) {
												}
												else if(next_value < value) {
												}
											}*/
											rTs.GetValue(j+duration, vec_idx, &next_value);
											const double result_delta = (next_value - value) / margin;
											if(result_delta > 0.0) {
												prob_result_entry.WinCount++;
												prob_result_entry.WinSum += result_delta;
											}
											else {
												prob_result_entry.LossCount++;
												prob_result_entry.LossSum += -result_delta;
											}
										}
									}
								}
								prob_result_list.insert(&prob_result_entry);
								stat_trend.Finish();
								msg_buf.Z().
									Cat(mean_mult, MKSFMTD(0, 3, 0)).
									Tab().Cat(duration).
									//Tab().Cat(target, MKSFMTD(0, 5, 0)).
									//Tab().Cat(duck, MKSFMTD(0, 5, 0)).
									Tab().Cat(prob_result_entry.CaseCount).
									Tab().Cat(stat_trend.GetExp(), MKSFMTD(0, 9, 0)).
									Tab().Cat(prob_result_entry.WinCount).
									Tab().Cat(prob_result_entry.WinSum, MKSFMTD(0, 5, 0)).
									Tab().Cat(prob_result_entry.LossCount).
									Tab().Cat(prob_result_entry.LossSum, MKSFMTD(0, 5, 0)).
									Tab().Cat((int)prob_result_entry.WinCount-(int)prob_result_entry.LossCount).
									Tab().Cat(prob_result_entry.WinSum-prob_result_entry.LossSum, MKSFMTD(0, 5, 0));
								f_out.WriteLine(msg_buf.CR());
							//}
						//}
					}
				}
				{
					int    best_count = -1000000;
					double best_result = -1E9;
					uint   best_count_idx = 0;
					uint   best_result_idx = 0;
					for(uint bi = 0; bi < prob_result_list.getCount(); bi++) {
						const __TsProbEntry & r_pe = prob_result_list.at(bi);
						int c = ((int)r_pe.WinCount-(int)r_pe.LossCount);
						double r = (r_pe.WinSum-r_pe.LossSum);
						if(best_count < c) {
							best_count = c;
							best_count_idx = bi;
						}
						if(best_result < r) {
							best_result = r;
							best_result_idx = bi;
						}
					}
					const __TsProbEntry & r_pe_best_count = prob_result_list.at(best_count_idx);
					const __TsProbEntry & r_pe_best_result = prob_result_list.at(best_result_idx);
					msg_buf.Z().Cat("The best count").Tab().
						Cat(r_pe_best_count.MeanMult).
						Tab().Cat(r_pe_best_count.Duration).
						//Tab().Cat(target, MKSFMTD(0, 5, 0)).
						//Tab().Cat(duck, MKSFMTD(0, 5, 0)).
						Tab().Cat(r_pe_best_count.CaseCount).
						Tab().Cat("").
						Tab().Cat(r_pe_best_count.WinCount).
						Tab().Cat(r_pe_best_count.WinSum, MKSFMTD(0, 5, 0)).
						Tab().Cat(r_pe_best_count.LossCount).
						Tab().Cat(r_pe_best_count.LossSum, MKSFMTD(0, 5, 0)).
						Tab().Cat((int)r_pe_best_count.WinCount-(int)r_pe_best_count.LossCount).
						Tab().Cat(r_pe_best_count.WinSum-r_pe_best_count.LossSum, MKSFMTD(0, 5, 0));
					f_out.WriteLine(msg_buf.CR());
					//
					msg_buf.Z().Cat("The best result").Tab().
						Cat(r_pe_best_result.MeanMult).
						Tab().Cat(r_pe_best_result.Duration).
						//Tab().Cat(target, MKSFMTD(0, 5, 0)).
						//Tab().Cat(duck, MKSFMTD(0, 5, 0)).
						Tab().Cat(r_pe_best_result.CaseCount).
						Tab().Cat("").
						Tab().Cat(r_pe_best_result.WinCount).
						Tab().Cat(r_pe_best_result.WinSum, MKSFMTD(0, 5, 0)).
						Tab().Cat(r_pe_best_result.LossCount).
						Tab().Cat(r_pe_best_result.LossSum, MKSFMTD(0, 5, 0)).
						Tab().Cat((int)r_pe_best_result.WinCount-(int)r_pe_best_result.LossCount).
						Tab().Cat(r_pe_best_result.WinSum-r_pe_best_result.LossSum, MKSFMTD(0, 5, 0));
					f_out.WriteLine(msg_buf.CR());
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPObjTimeSeries::ProcessNN(const STimeSeries & rTs, const TrainNnParam & rP)
{
	int    ok = 1;
	const  uint tsc = rTs.GetCount();
	float  nn_outp[3];
	const  bool have_maxduck = (rP.MaxDuck > 0.0);
	const  bool have_target  = (rP.Target > 0.0);
	const  uint input_frame_size = rP.InputFrameSize/*500*/;
	const  long input_dim = (long)input_frame_size;
	const  long outp_dim = SIZEOFARRAY(nn_outp);
	const  double diff_scale = (rP.Margin != 0.0) ? (1.0 / rP.Margin) : 0.0;
	SString msg_buf;
	SString temp_buf;
	SString save_file_name;
	SString test_file_name;
	SString anlz_file_name;
	rP.MakeFileName(temp_buf);
	PPGetFilePath(PPPATH_OUT, temp_buf, save_file_name);
	SPathStruc::ReplaceExt(temp_buf, "fann-test", 1);
	PPGetFilePath(PPPATH_OUT, temp_buf, test_file_name);
	temp_buf = "AnalyzeTsTradeFrames.txt";
	PPGetFilePath(PPPATH_OUT, temp_buf, anlz_file_name);

	SFile * p_anlz_file = 0;
	float * p_inp_vec = 0;
	struct fann * p_ann = 0;
	fann_train_data * p_train_data = 0;
	uint   vec_idx = 0;
	THROW_SL(rTs.GetValueVecIndex("close", &vec_idx));
	{
		msg_buf.Z().Cat(rP.Symb).Space().CatEq("ForwardFrame", rP.ForwardFrameSize).
			Space().CatEq("Target", rP.Target, MKSFMTD(0, 8, 0)).
			Space().CatEq("MaxDuck", rP.MaxDuck, MKSFMTD(0, 8, 0)).
			Space().CatEq("InputFrame", rP.InputFrameSize).
			Space().CatEq("HiddenLayer", rP.HiddenLayerDim).
			Space().CatEq("LearningRate", rP.LearningRate, MKSFMTD(0, 3, 0));
		PPLogMessage(PPFILNAM_NNTRAIN_LOG, msg_buf, LOGMSGF_TIME);
	}
	if(tsc > (input_frame_size + rP.ForwardFrameSize)) {
		RealArray inp_frame;
		RealArray result_frame;
		uint fs;
		uint total_set_count = 0; // Количество наборов данных

		uint target_count = 0;
		uint duck_count = 0;
		StatBase max_target_stat;
		StatBase max_duck_stat;
		if(rP.Flags & rP.fAnalyzeFrame) {
			const int is_anlz_file_exists = fileExists(anlz_file_name);
			THROW_MEM(p_anlz_file = new SFile(anlz_file_name, SFile::mAppend));
			if(!is_anlz_file_exists) {
				msg_buf.Z().Cat(getcurdatetime_(), DATF_YMD|DATF_CENTURY, TIMF_HMS);
				p_anlz_file->WriteLine(msg_buf.CR());
				msg_buf.Z().Cat("symb").
					Tab().Cat("frame").
					Tab().Cat("set_count").
					Tab().Cat("target").
					Tab().Cat("duck").
					Tab().Cat("target_freq").
					Tab().Cat("duck_freq").
					Tab().Cat("max_target_avg").
					Tab().Cat("max_duck_avg");
				p_anlz_file->WriteLine(msg_buf.CR());
			}
		}
		for(fs = input_frame_size; fs < (tsc - input_frame_size - rP.ForwardFrameSize); fs++) {
			//rTs.GetFrame(vec_idx, fs-input_frame_size, input_frame_size, diff_scale, STimeSeries::nfZero|STimeSeries::nfBaseStart, inp_frame);
			//if(inp_frame.getCount() == input_frame_size) {
				if((fs+rP.ForwardFrameSize) <= tsc) {
					total_set_count++;
					if(rP.Flags & rP.fAnalyzeFrame) {
						double last_signal_val = 0.0;
						rTs.GetValue(fs, vec_idx, &last_signal_val);
						int   target_reached = 0;
						int   max_duck_reached = 0;
						double max_target = 0.0;
						double max_duck = 0.0;
						double last_result_val = 0.0;
						rTs.GetValue(fs+rP.ForwardFrameSize-1, vec_idx, &last_result_val);
						for(uint i = 0; !target_reached && !max_duck_reached && i < rP.ForwardFrameSize; i++) {
							double fv;
							assert(rTs.GetValue(fs+i, vec_idx, &fv));
							double fdiff = fv - last_signal_val;
							if(diff_scale != 0.0)
								fdiff *= diff_scale;
							if(fdiff < 0.0) {
								if(!target_reached) {
									SETMAX(max_duck, -fdiff);
								}
								if(have_maxduck && fdiff < -rP.MaxDuck) {
									if(!max_duck_reached)
										duck_count++;
									max_duck_reached = 1;
								}
							}
							else { 
								if(!max_duck_reached) {
									SETMAX(max_target, fdiff);
								}
								if(have_target && fdiff > rP.Target) {
									if(!target_reached)
										target_count++;
									target_reached = 1;
								}
							}
						}
						max_target_stat.Step(max_target);
						max_duck_stat.Step(max_duck);
					}
				}
			//}
		}
		if(p_anlz_file) {
			max_target_stat.Finish();
			max_duck_stat.Finish();
			const double target_freq = ((double)target_count) / ((double)total_set_count);
			const double duck_freq = ((double)duck_count) / ((double)total_set_count);
			msg_buf.Z().Cat(rP.Symb).
				Tab().Cat(rP.ForwardFrameSize).
				Tab().Cat(total_set_count).
				Tab().Cat(rP.Target, MKSFMTD(0, 4, 0)).
				Tab().Cat(rP.MaxDuck, MKSFMTD(0, 4, 0)).
				Tab().Cat(target_freq, MKSFMTD(0, 8, 0)).
				Tab().Cat(duck_freq, MKSFMTD(0, 8, 0)).
				Tab().Cat(max_target_stat.GetExp(), MKSFMTD(0, 8, 0)).
				Tab().Cat(max_duck_stat.GetExp(), MKSFMTD(0, 8, 0));
			p_anlz_file->WriteLine(msg_buf.CR());
			ZDELETE(p_anlz_file);
		}
		if(rP.Flags & (rP.fTeach|rP.fTest)) {
			if(fileExists(save_file_name)) {
				p_ann = fann_create_from_file(save_file_name);
				if(p_ann) {
					msg_buf.Z().Cat("NN has been created from file").Space().Cat(save_file_name);
					PPLogMessage(PPFILNAM_NNTRAIN_LOG, msg_buf, LOGMSGF_TIME);
				}
			}
			if(!p_ann) {
				p_ann = fann_create_standard_2(3, input_dim, rP.HiddenLayerDim, outp_dim);
				THROW(p_ann);
				fann_set_activation_function_hidden(p_ann, FANN_SIGMOID);
				fann_set_activation_function_output(p_ann, FANN_SIGMOID);
				fann_randomize_weights(p_ann, -1.0f, 1.0f);
			}
			fann_set_learning_rate(p_ann, rP.LearningRate);
			fann_set_training_algorithm(p_ann, /*FANN_TRAIN_QUICKPROP*/FANN_TRAIN_INCREMENTAL);
			fs = input_frame_size;
			PPLogMessage(PPFILNAM_NNTRAIN_LOG, msg_buf.Z().CatEq("total_set_count", total_set_count), LOGMSGF_TIME);
			if(rP.Flags & rP.fTeach) {
				const uint max_set_count_chunk = 1000;
				for(uint set_rest = total_set_count, step_no = 0; set_rest > 0; step_no++) {
					const  uint set_count = MIN(max_set_count_chunk, set_rest);
					uint   set_idx = 0; // Индекс текущего заполняемого набора данных
					uint   target_count = 0;
					uint   duck_count = 0;
					THROW(p_train_data = fann_create_train(set_count, input_dim, outp_dim));
					for(; set_idx < set_count && fs < (tsc - input_frame_size - rP.ForwardFrameSize); fs++) {
						rTs.GetFrame(vec_idx, fs-input_frame_size, input_frame_size, diff_scale, STimeSeries::nfZero|STimeSeries::nfBaseStart, inp_frame);
						if(inp_frame.getCount() == input_frame_size) {
							double last_signal_val = 0.0;
							rTs.GetValue(fs, vec_idx, &last_signal_val);
							//rTs.GetFrame(vec_idx, fs, rP.ForwardFrameSize, diff_scale, STimeSeries::nfZero|STimeSeries::nfBaseStart, result_frame);
							//if(result_frame.getCount() == rP.ForwardFrameSize) {
							if((fs+rP.ForwardFrameSize) <= tsc) {
								int   target_reached = 0;
								int   max_duck_reached = 0;
								double last_result_val = 0.0;
								rTs.GetValue(fs+rP.ForwardFrameSize-1, vec_idx, &last_result_val);
								for(uint iidx = 0; iidx < input_frame_size; iidx++) {
									//nn_inp[iidx] = (float)inp_frame.at(iidx);
									p_train_data->input[set_idx][iidx] = (float)inp_frame.at(iidx);
								}
								/*
								for(uint i = 0; !target_reached && !max_duck_reached && i < result_frame.getCount(); i++) {
									const double fv = result_frame.at(i);
									if(fv > rP.Target)
										target_reached = 1;
									else if(fv < rP.MaxDuck)
										max_duck_reached = 1;
								}
								*/
								for(uint i = 0; !target_reached && !max_duck_reached && i < rP.ForwardFrameSize; i++) {
									double fv;
									assert(rTs.GetValue(fs+i, vec_idx, &fv));
									double fdiff = fv - last_signal_val;
									if(diff_scale != 0.0)
										fdiff *= diff_scale;
									if(fdiff > rP.Target)
										target_reached = 1;
									else if(fdiff < -rP.MaxDuck)
										max_duck_reached = 1;
								}
								if(target_reached) {
									nn_outp[0] = 1.0f;
									nn_outp[1] = 0.0f;
									nn_outp[2] = 0.0f;
									target_count++;
								}
								else if(max_duck_reached) {
									nn_outp[0] = 0.0f;
									nn_outp[1] = 1.0f;
									nn_outp[2] = 0.0f;
									duck_count++;
								}
								else {
									nn_outp[0] = 0.0f;
									nn_outp[1] = 0.0f;
									nn_outp[2] = 1.0f;
								}
								p_train_data->output[set_idx][0] = nn_outp[0];
								p_train_data->output[set_idx][1] = nn_outp[1];
								p_train_data->output[set_idx][2] = nn_outp[2];
								set_idx++;
							}
						}
					}
					//
					{
						float train_error = 0.0f;
						for(uint train_epoch = 0; train_epoch < rP.EpochCount; train_epoch++) {
							train_error = fann_train_epoch(p_ann, p_train_data);
						}
						if(step_no && (step_no % 10) == 0)
							fann_save(p_ann, save_file_name);
						msg_buf.Z().Cat(total_set_count-set_rest+set_count).Space().Cat("sets is passed").Semicol().
							CatEq("train_error", train_error, MKSFMTD(0, 8, 0)).Space().CatEq("target_count", target_count);
						PPLogMessage(PPFILNAM_NNTRAIN_LOG, msg_buf, LOGMSGF_TIME);
					}
					fann_destroy_train(p_train_data);
					p_train_data = 0;
					assert(set_idx == set_count);
					set_rest -= set_count;
				}
				if(fann_save(p_ann, save_file_name) == 0) {
					msg_buf.Z().Cat("NN has been saved to file").Space().Cat(save_file_name);
					PPLogMessage(PPFILNAM_NNTRAIN_LOG, msg_buf, LOGMSGF_TIME);
				}
			}
			if(rP.Flags & rP.fTest) {
				//double total_result = 0.0;
				SFile f_out_test(test_file_name, SFile::mWrite);
				THROW_MEM(p_inp_vec = new float[input_frame_size]);
				memzero(p_inp_vec, sizeof(*p_inp_vec) * input_frame_size);
				for(fs = input_frame_size; fs < (tsc - input_frame_size - rP.ForwardFrameSize); fs++) {
					rTs.GetFrame(vec_idx, fs-input_frame_size, input_frame_size, STimeSeries::nfOne|STimeSeries::nfBaseStart, inp_frame);
					if(inp_frame.getCount() == input_frame_size) {
						double last_signal_val = 0.0;
						rTs.GetValue(fs, vec_idx, &last_signal_val);
						for(uint iidx = 0; iidx < input_frame_size; iidx++) {
							p_inp_vec[iidx] = (float)inp_frame.at(iidx);
						}
						const float * p_outp = fann_run(p_ann, p_inp_vec);
						if(p_outp) {
							msg_buf.Z();
							if(p_outp[0] > 0.5f) {
								msg_buf.Cat("bingo").Space();
							}
							msg_buf.CatChar('[').Cat(p_outp[0], MKSFMTD(0, 8, 0)).CatChar(']').
								CatChar('[').Cat(p_outp[1], MKSFMTD(0, 8, 0)).CatChar(']').
								CatChar('[').Cat(p_outp[2], MKSFMTD(0, 8, 0)).CatChar(']');
							//msg_buf.Cat(p_outp[0], MKSFMTD(0, 8, 0));
							if((fs+rP.ForwardFrameSize) <= tsc) {
								double last_result_val = 0.0;
								rTs.GetValue(fs+rP.ForwardFrameSize-1, vec_idx, &last_result_val);
								//rTs.GetFrame("close", fs, rP.ForwardFrameSize, STimeSeries::nfOne|STimeSeries::nfBaseStart, result_frame);
								//if(result_frame.getCount() == rP.ForwardFrameSize) {

								double result = (last_result_val - last_signal_val) * 10000.0;
								msg_buf.Tab().Cat(result, MKSFMTD(0, 2, 0));
								//msg_buf.CatChar(';').Cat(result, MKSFMTD(0, 2, 0));
							}
							/*else
								msg_buf.Tab().Cat("series finished");*/
							f_out_test.WriteLine(msg_buf.CR());
						}
					}
				}
			}
		}
	}
	CATCH
		PPLogMessage(PPFILNAM_NNTRAIN_LOG, 0, LOGMSGF_LASTERR|LOGMSGF_TIME);
		ok = 0;
	ENDCATCH
	fann_destroy_train(p_train_data);
	fann_destroy(p_ann);
	delete [] p_inp_vec;
	delete p_anlz_file;
	return ok;
}

/*
time-series-precision: точность представления значений
time-series-margin-buy:  маржина при покупке инструмента, представленного символом
time-series-margin-sell: маржина при продаже инструмента, представленного символом
time-series-currency-base: основная валюта
time-series-currency-profit: валюта прибыли
time-series-currency-margin: валюта маржины
*/