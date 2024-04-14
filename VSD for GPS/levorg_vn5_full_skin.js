//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Height / 720;
	
	// Google Maps パラメータ設定
	GoogleMapsParam = {
		// ズームレベルを 0～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真
		Maptype: "roadmap",
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		8 * Scale,
		Y:		8 * Scale,
		Width:	Math.min( 300 * Scale, 640 ),
		Height:	Math.min( 300 * Scale, 640 ),
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
		
		// 地図更新間隔
		// 前回地図更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみ地図を更新します
		UpdateTime:		1000,	// [ミリ秒]
		UpdateDistance:	10,		// [ピクセル]
	};
	
	// Geocoding の設定
	GeocodingParam = {
		// Geocoding 更新間隔
		UpdateTime:	10000,	// [ミリ秒]
	};
	
	if( GoogleAPIKey[ 0 ] == '' ){
		if( MessageBox(
			"本スキンを使用するためには初期設定が必要です．詳しくは\n" +
			"VSD for GPS インストール手順の web ページを参照してください．\n" +
			"(OK を押すと web ブラウザを開きます)",
			undefined,
			MB_OKCANCEL | MB_ICONINFORMATION
		) == IDOK ){
			var WshShell = new ActiveXObject( "WScript.Shell" );
			WshShell.Run( "cmd /c start https://yoshinrt.github.io/vsd/install#GoogleAPIKey" );
		}
	}
	
	// 使用する画像・フォントの宣言
	FontS = new Font( "Impact", 24 * Scale );
	FontJ = new Font( "ＭＳ Ｐゴシック", 31 * Scale );
	FontM = new Font( "Impact", 31 * Scale, FONT_FIXED );
	FontL = new Font( "Impact", 60 * Scale );
	
	// アングルメータ用最大角計算
	// MaxAngle = Math.abs(Log.Min.Angle)>Log.Max.Angle?Math.abs(Log.Min.Angle):Log.Max.Angle;
	MaxAngle = 30

	// スピードメータ用最高速計算
	// MaxSpeed = Math.ceil( Log.Max.Speed / 10 ) * 10;
	MaxSpeed = Math.ceil( 180 / 10 ) * 10;
	
	// タコメータ用最高回転数計算
	// MaxTacho = Math.ceil( Log.Max.Tacho / 1000 );
	MaxTacho = Math.ceil( 6000 / 1000 );
	
	// ブレーキメータ用最高値計算
	// MaxBrake = Math.ceil( Log.Max.Brake / 1000 );
	MaxBrake = 100

	// アクセルメータ用最高値計算
	// MaxAccel = Math.ceil( Log.Max.Accel / 1000 );
	MaxAccel = 100

	// 座標等を予め計算しておく
	MeterR  = 120 * Scale;

	MeterMin = 170;
	MeterMax = 10;
	MeterCenter = 270;
	MeterOffsetY = MeterR * 5 / 8;

	AngleMeterParam = {
		X:		Vsd.Width  * 1 / 8,
		Y:		Vsd.Height - MeterOffsetY,
		R:		MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	5,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	6,
		Line2Len:	MeterR * 0.08,
		Line2Width:	4,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	10,
		NumR:		MeterR * 0.78,
		FontColor:	0x00FFFFFF,
		Font:		FontS,
		MinAngle:	MeterMin,
		MaxAngle:	MeterMax,
		MinVal:		- MaxAngle,
		MaxVal:		MaxAngle,
	};

	SpeedMeterParam = {
		X:		Vsd.Width  * 3 / 8,
		Y:		Vsd.Height - MeterOffsetY,
		R:		MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	5,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	9,
		Line2Len:	MeterR * 0.08,
		Line2Width:	4,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	4,
		NumR:		MeterR * 0.78,
		FontColor:	0xFFFFFF,
		Font:		FontS,
		MinAngle:	MeterMin,
		MaxAngle:	MeterMax,
		MinVal:		0,
		MaxVal:		MaxSpeed,
	};
	
	TachoMeterParam = {
		X:		Vsd.Width  * 5 / 8,
		Y:		Vsd.Height - MeterOffsetY,
		R:		MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	5,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	6,
		Line2Len:	MeterR * 0.08,
		Line2Width:	4,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	2,
		NumR:		MeterR * 0.78,
		FontColor:	0xFFFFFF,
		Font:		FontS,
		MinAngle:	MeterMin,
		MaxAngle:	MeterMax,
		MinVal:		0,
		MaxVal:		MaxTacho,
	};

	BrakeMeterParam = {
		X:			Vsd.Width  * 7 / 8,
		Y:			Vsd.Height - MeterOffsetY,
		R:			MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	5,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	1,
		Line2Len:	MeterR * 0.08,
		Line2Width:	4,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	10,
		NumR:		MeterR * 0.78,
		FontColor:	0xFFFFFFFF,
		Font:		FontS,
		MinAngle:	MeterMin,
		MaxAngle:	MeterCenter,
		MinVal:		0,
		MaxVal:		MaxBrake,
	};
	
	AccelMeterParam = {
		X:			Vsd.Width  * 7 / 8,
		Y:			Vsd.Height - MeterOffsetY,
		R:			MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	5,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	1,
		Line2Len:	MeterR * 0.08,
		Line2Width:	4,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	10,
		NumR:		MeterR * 0.78,
		FontColor:	0xFFFFFFFF,
		Font:		FontS,
		MinAngle:	MeterCenter,
		MaxAngle:	MeterMax,
		MinVal:		0,
		MaxVal:		MaxAccel,
	};
	
	BGColor = 0x80001020;
}

//*** メーター描画処理 ******************************************************

function Draw(){

	//////////     アングルメーター     //////////
	
	// アングルメーター目盛り描画
	Vsd.DrawRoundMeterScale( AngleMeterParam );

	// アングルメーター針

	Vsd.DrawNeedle(
		AngleMeterParam.X, AngleMeterParam.Y, MeterR, MeterR * 0.93,
		MeterMin, MeterMax, (AngleMeterParam.MaxVal + Log.Angle) / (AngleMeterParam.MaxVal * 2), 0xFF0000, 10
	);

	// アングル数値表示
	Vsd.DrawTextAlign(
		AngleMeterParam.X, AngleMeterParam.Y , 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Log.Angle, FontL, 0xFFFFFF
	);
	
	// アングル単位表示
	Vsd.DrawTextAlign(
		AngleMeterParam.X, AngleMeterParam.Y - MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_TOP,
		"STEERLING", FontS, 0xFFFFFF
	);

	//////////     スピードメーター     //////////
	
	// スピードメーター目盛り描画
	Vsd.DrawRoundMeterScale( SpeedMeterParam );
	
	// スピードメーター針
	Vsd.DrawNeedle(
		SpeedMeterParam.X, SpeedMeterParam.Y, MeterR, MeterR * 0.93,
		MeterMin, MeterMax, Log.Speed / SpeedMeterParam.MaxVal, 0xFF0000, 10
	);

	// スピード数値表示
	Vsd.DrawTextAlign(
		SpeedMeterParam.X, SpeedMeterParam.Y , 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Log.Speed, FontL, 0xFFFFFF
	);
	
	// スピード単位表示
	Vsd.DrawTextAlign(
		SpeedMeterParam.X, SpeedMeterParam.Y - MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_TOP,
		"km/h", FontS, 0xFFFFFF
	);

	//////////     タコメーター     //////////
	
	// タコメーター目盛り描画
	Vsd.DrawRoundMeterScale( TachoMeterParam );
	
	// タコメーター針
	Vsd.DrawNeedle(
		TachoMeterParam.X, TachoMeterParam.Y, MeterR, MeterR * 0.93,
		MeterMin, MeterMax, (Log.Tacho / 1000) / TachoMeterParam.MaxVal, 0xFF0000, 10
	);

	// タコ数値表示
	Vsd.DrawTextAlign(
		TachoMeterParam.X, TachoMeterParam.Y , 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Log.Tacho, FontL, 0xFFFFFF
	);
	
	// タコ単位表示
	Vsd.DrawTextAlign(
		TachoMeterParam.X, TachoMeterParam.Y - MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_TOP,
		"x1000RPM", FontS, 0xFFFFFF
	);
	
	//////////     アクセル/ブレーキメーター     //////////

	// ブレーキメーター針
	Vsd.DrawArc(
		BrakeMeterParam.X, BrakeMeterParam.Y, MeterR, MeterR,
		MeterR * 0.93, MeterR * 0.93,
		(MeterCenter - 1) - (Log.Brake / BrakeMeterParam.MaxVal * (MeterCenter - MeterMin - 1)),
		MeterCenter, 0xFF0000
	);
	
	// アクセルメーター針
	if((360 - MeterCenter - 1) <= Log.Accel / Log.Max.Accel * (MeterCenter - MeterMin - 1)){
		Vsd.DrawArc(
			AccelMeterParam.X, AccelMeterParam.Y, MeterR, MeterR,
			MeterR * 0.93, MeterR * 0.93, MeterCenter,
			(Log.Accel / AccelMeterParam.MaxVal * (MeterCenter - MeterMin - 1)) - (360 - MeterCenter - 1), 0x0000FF
		);
	} else {
		Vsd.DrawArc(
			AccelMeterParam.X, AccelMeterParam.Y, MeterR, MeterR,
			MeterR * 0.93, MeterR * 0.93, MeterCenter,
			(Log.Accel / AccelMeterParam.MaxVal * (MeterCenter - MeterMin - 1)) + MeterCenter + 1 , 0x0000FF
		);
	}
	
	// ブレーキメーター目盛り描画
	Vsd.DrawRoundMeterScale( BrakeMeterParam );

	Vsd.DrawTextAlign(
		BrakeMeterParam.X + BrakeMeterParam.NumR * Math.cos(Math.PI * (360 - MeterMin) / 180),
		BrakeMeterParam.Y - BrakeMeterParam.NumR * Math.sin(Math.PI * (360 - MeterMin) / 180),
		ALIGN_LEFT | ALIGN_VCENTER,
		"Brake", BrakeMeterParam.Font, 0xFFFFFF
	);

	// アクセルメーター目盛り描画
	Vsd.DrawRoundMeterScale( AccelMeterParam );

	Vsd.DrawTextAlign(
		AccelMeterParam.X + AccelMeterParam.NumR * Math.cos(Math.PI * (360 - MeterMax) / 180),
		AccelMeterParam.Y - AccelMeterParam.NumR * Math.sin(Math.PI * (360 - MeterMax) / 180),
		ALIGN_RIGHT | ALIGN_VCENTER,
		"Accel", AccelMeterParam.Font, 0xFFFFFF
	);

	// シフト文字表示
	switch(Log.Shift){
		case 4:
			Vsd.DrawTextAlign(
				BrakeMeterParam.X, BrakeMeterParam.Y , 
				ALIGN_HCENTER | ALIGN_VCENTER,
				"P", FontL, 0xFFFFFF
			);
			break;
		case 3:
			Vsd.DrawTextAlign(
				BrakeMeterParam.X, BrakeMeterParam.Y , 
				ALIGN_HCENTER | ALIGN_VCENTER,
				"R", FontL, 0xFFFFFF
			);
			break;
		case 2:
			Vsd.DrawTextAlign(
				BrakeMeterParam.X, BrakeMeterParam.Y , 
				ALIGN_HCENTER | ALIGN_VCENTER,
				"N", FontL, 0xFFFFFF
			);
			break;
		case 1:
			Vsd.DrawTextAlign(
				BrakeMeterParam.X, BrakeMeterParam.Y , 
				ALIGN_HCENTER | ALIGN_VCENTER,
				"D", FontL, 0xFFFFFF
			);
			break;
	}
	
	// シフト単位表示
	Vsd.DrawTextAlign(
		BrakeMeterParam.X, BrakeMeterParam.Y - MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_TOP,
		"SHIFT", FontS, 0xFFFFFF
	);

	Vsd.DrawRect( 0, 0, 316 * Scale - 1, 316 * Scale - 1, BGColor, DRAW_FILL );
	
	// Google マップ表示
	Vsd.DrawGoogleMaps( GoogleMapsParam );
	
}
