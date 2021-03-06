<?php
/*
** Zabbix
** Copyright (C) 2001-2018 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/


$widget = (new CWidget())->setTitle(_('Maintenance periods'));

// create form
$maintenanceForm = (new CForm())
	->setName('maintenanceForm')
	->setAttribute('aria-labeledby', ZBX_STYLE_PAGE_TITLE)
	->addVar('form', $this->data['form']);
if (isset($this->data['maintenanceid'])) {
	$maintenanceForm->addVar('maintenanceid', $this->data['maintenanceid']);
}

/*
 * Maintenance tab
 */
$maintenanceFormList = (new CFormList('maintenanceFormList'))
	->addRow(
		(new CLabel(_('Name'), 'mname'))->setAsteriskMark(),
		(new CTextBox('mname', $this->data['mname']))
			->setWidth(ZBX_TEXTAREA_STANDARD_WIDTH)
			->setAriaRequired()
			->setAttribute('autofocus', 'autofocus')
			->setAttribute('maxlength', DB::getFieldLength('maintenances', 'name'))
	)
	->addRow((new CLabel(_('Maintenance type'), 'maintenance_type')),
		(new CRadioButtonList('maintenance_type', (int) $data['maintenance_type']))
			->addValue(_('With data collection'), MAINTENANCE_TYPE_NORMAL)
			->addValue(_('No data collection'), MAINTENANCE_TYPE_NODATA)
			->setModern(true)
	);

// active since
if (isset($_REQUEST['active_since'])) {
	$fromYear = getRequest('active_since_year');
	$fromMonth = getRequest('active_since_month');
	$fromDay = getRequest('active_since_day');
	$fromHours = getRequest('active_since_hour');
	$fromMinutes = getRequest('active_since_minute');
	$fromDate = [
		'y' => $fromYear,
		'm' => $fromMonth,
		'd' => $fromDay,
		'h' => $fromHours,
		'i' => $fromMinutes
	];
	$activeSince = $fromYear.$fromMonth.$fromDay.$fromHours.$fromMinutes;
}
else {
	$fromDate = zbxDateToTime($this->data['active_since']);
	$activeSince = $this->data['active_since'];
}
$maintenanceForm->addVar('active_since', $activeSince);

// active till
if (isset($_REQUEST['active_till'])) {
	$toYear = getRequest('active_till_year');
	$toMonth = getRequest('active_till_month');
	$toDay = getRequest('active_till_day');
	$toHours = getRequest('active_till_hour');
	$toMinutes = getRequest('active_till_minute');
	$toDate = [
		'y' => $toYear,
		'm' => $toMonth,
		'd' => $toDay,
		'h' => $toHours,
		'i' => $toMinutes,
	];
	$activeTill = $toYear.$toMonth.$toDay.$toHours.$toMinutes;
}
else {
	$toDate = zbxDateToTime($this->data['active_till']);
	$activeTill = $this->data['active_till'];
}
$maintenanceForm->addVar('active_till', $activeTill);

$maintenanceFormList->addRow(_('Active since'), createDateSelector('active_since', $fromDate, 'active_till'));
$maintenanceFormList->addRow(_('Active till'), createDateSelector('active_till', $toDate, 'active_since'));

$maintenanceFormList->addRow(_('Description'),
	(new CTextArea('description', $this->data['description']))->setWidth(ZBX_TEXTAREA_STANDARD_WIDTH)
);

/*
 * Maintenance period tab
 */
$maintenancePeriodFormList = new CFormList('maintenancePeriodFormList');
$maintenance_period_table = (new CTable())
	->setAttribute('style', 'width: 100%;')
	->setHeader([_('Period type'), _('Schedule'), _('Period'), _('Action')])
	->setId('maintenance_periods')
	->setAriaRequired();

foreach ($this->data['timeperiods'] as $id => $timeperiod) {
	$maintenance_period_table->addRow([
		(new CCol(timeperiod_type2str($timeperiod['timeperiod_type'])))->addClass(ZBX_STYLE_NOWRAP),
		shedule2str($timeperiod),
		(new CCol(zbx_date2age(0, $timeperiod['period'])))->addClass(ZBX_STYLE_NOWRAP),
		(new CCol(
			new CHorList([
				(new CSimpleButton(_('Edit')))
					->onClick('javascript: submitFormWithParam('.
						'"'.$maintenanceForm->getName().'", "edit_timeperiodid['.$id.']", "1"'.
					');')
					->addClass(ZBX_STYLE_BTN_LINK),
				(new CSimpleButton(_('Remove')))
					->onClick('javascript: submitFormWithParam('.
						'"'.$maintenanceForm->getName().'", "del_timeperiodid['.$id.']", "1"'.
					');')
					->addClass(ZBX_STYLE_BTN_LINK)
			])
		))->addClass(ZBX_STYLE_NOWRAP)
	]);
	if (isset($timeperiod['timeperiodid'])) {
		$maintenanceForm->addVar('timeperiods['.$id.'][timeperiodid]', $timeperiod['timeperiodid']);
	}
	$maintenanceForm
		->addVar('timeperiods['.$id.'][timeperiod_type]', $timeperiod['timeperiod_type'])
		->addVar('timeperiods['.$id.'][every]', $timeperiod['every'])
		->addVar('timeperiods['.$id.'][month]', $timeperiod['month'])
		->addVar('timeperiods['.$id.'][dayofweek]', $timeperiod['dayofweek'])
		->addVar('timeperiods['.$id.'][day]', $timeperiod['day'])
		->addVar('timeperiods['.$id.'][start_time]', $timeperiod['start_time'])
		->addVar('timeperiods['.$id.'][start_date]', $timeperiod['start_date'])
		->addVar('timeperiods['.$id.'][period]', $timeperiod['period']);
}

$periodsDiv = (new CDiv($maintenance_period_table))
	->addClass(ZBX_STYLE_TABLE_FORMS_SEPARATOR)
	->setAttribute('style', 'min-width: '.ZBX_TEXTAREA_BIG_WIDTH.'px;');

if (!isset($_REQUEST['new_timeperiod'])) {
	$periodsDiv->addItem(
		(new CSimpleButton(_('New')))
			->onClick('javascript: submitFormWithParam("'.$maintenanceForm->getName().'", "new_timeperiod", "1");')
			->addClass(ZBX_STYLE_BTN_LINK)
	);
}
$maintenancePeriodFormList->addRow(
	(new CLabel(_('Periods'), $maintenance_period_table->getId()))->setAsteriskMark(), $periodsDiv
);

if (isset($_REQUEST['new_timeperiod'])) {
	if (is_array($_REQUEST['new_timeperiod']) && isset($_REQUEST['new_timeperiod']['id'])) {
		$saveLabel = _('Update');
	}
	else {
		$saveLabel = _('Add');
	}

	$maintenancePeriodFormList->addRow(_('Maintenance period'),
		(new CDiv([
			get_timeperiod_form(),
			new CHorList([
				(new CSimpleButton($saveLabel))
					->onClick('javascript: submitFormWithParam("'.$maintenanceForm->getName().'", "add_timeperiod", "1");')
					->addClass(ZBX_STYLE_BTN_LINK),
				(new CSimpleButton(_('Cancel')))
					->onClick('javascript: submitFormWithParam("'.$maintenanceForm->getName().'", "cancel_new_timeperiod", "1");')
					->addClass(ZBX_STYLE_BTN_LINK)
			])
		]))
			->addClass(ZBX_STYLE_TABLE_FORMS_SEPARATOR)
			->setAttribute('style', 'min-width: '.ZBX_TEXTAREA_BIG_WIDTH.'px;')
	);
}

// Hosts and groups tab.
$hostsAndGroupsFormList = (new CFormList('hostsAndGroupsFormList'))
	->addRow('',
		(new CLabel(_('At least one host or host group must be selected.')))->setAsteriskMark()
	)
	->addRow(new CLabel(_('Hosts in maintenance'), 'hosts[]'),
		(new CMultiSelect([
			'name' => 'hostids[]',
			'objectName' => 'hosts',
			'objectOptions' => [
				'editable' => true
			],
			'data' => $data['hosts_ms'],
			'popup' => [
				'parameters' => [
					'srctbl' => 'hosts',
					'dstfrm' => $maintenanceForm->getName(),
					'dstfld1' => 'hostids_',
					'srcfld1' => 'hostid',
					'writeonly' => '1',
					'multiselect' => '1'
				]
			]
		]))->setWidth(ZBX_TEXTAREA_STANDARD_WIDTH)
	)
	->addRow(new CLabel(_('Groups in maintenance'), 'groups[]'),
		(new CMultiSelect([
			'name' => 'groupids[]',
			'objectName' => 'hostGroup',
			'objectOptions' => [
				'editable' => true
			],
			'data' => $data['groups_ms'],
			'popup' => [
				'parameters' => [
					'srctbl' => 'host_groups',
					'dstfrm' => $maintenanceForm->getName(),
					'dstfld1' => 'groupids_',
					'srcfld1' => 'groupid',
					'writeonly' => '1',
					'multiselect' => '1'
				]
			]
		]))->setWidth(ZBX_TEXTAREA_STANDARD_WIDTH)
	);

// Append tabs to form.
$maintenanceTab = (new CTabView())
	->addTab('maintenanceTab', _('Maintenance'), $maintenanceFormList)
	->addTab('periodsTab', _('Periods'), $maintenancePeriodFormList)
	->addTab('hostTab', _('Hosts & Groups'), $hostsAndGroupsFormList);
if (!$this->data['form_refresh']) {
	$maintenanceTab->setSelected(0);
}

// append buttons to form
if (isset($this->data['maintenanceid'])) {
	$maintenanceTab->setFooter(makeFormFooter(
		new CSubmit('update', _('Update')),
		[
			new CSubmit('clone', _('Clone')),
			new CButtonDelete(_('Delete maintenance period?'), url_param('form').url_param('maintenanceid')),
			new CButtonCancel()
		]
	));
}
else {
	$maintenanceTab->setFooter(makeFormFooter(
		new CSubmit('add', _('Add')),
		[new CButtonCancel()]
	));
}

$maintenanceForm->addItem($maintenanceTab);

$widget->addItem($maintenanceForm);

return $widget;
