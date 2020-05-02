import {Component, OnInit} from '@angular/core';
import {FormBuilder, FormGroup, Validators} from '@angular/forms';
import {Router} from '@angular/router';

import {colorToHex, hexToColor} from '../../color-convert.module';
import {Action, SubAction} from '../action';
import {ActionService} from '../action.service';

@Component({
  selector: 'app-action-add',
  templateUrl: './action-add.component.html',
  styleUrls: [
    './action-add.component.css',
    '../action-details/action-details.component.css'
  ]
})
export class ActionAddComponent implements OnInit {
  subActions: SubAction[] = [];
  header: FormGroup;
  icons = [];

  constructor(
      private fb: FormBuilder, private actionService: ActionService,
      private router: Router) {}

  ngOnInit() {
    this.header = this.fb.group(
        {name: ['', Validators.required], color: ['#FFFFFF'], icon: ['']});
  }

  cancelAction() {
    this.router.navigate(['/actions'], {replaceUrl: true});
  }

  finishAction() {
    if (this.header.valid) {
      const {name, color, icon} = this.header.value;
      let action: Action = {
        name: name,
        color: hexToColor(color),
        icon: icon,
        id: 0,
        subActions: this.subActions,
        visible: true
      };
      this.actionService.addAction(action);
      // Replace url so that navigating back does not accidentally create
      // another action
      this.router.navigate(['/actions'], {replaceUrl: true});
    }
  }
}
