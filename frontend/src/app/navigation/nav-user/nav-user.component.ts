import { Component, OnInit } from '@angular/core';

import { User } from '../../user/user';
import { UserService } from '../../user/user.service';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-navigation-user',
  templateUrl: './nav-user.component.html',
  styleUrls: ['./nav-user.component.css']
})
export class NavUserComponent implements OnInit {

    user$: Observable<User>;

    constructor(private userService: UserService) { }

    ngOnInit() {
      this.getUser();
    }

    getUser(): void {
      this.user$ = this.userService.getUser();
    }
}
